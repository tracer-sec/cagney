import socket
import select
import threading
import time

'''
TODO
- error on duplicate bot ID
- graceful closing
'''

class Message(object):
    def __init__(self, message):
        parts = message.split('|', 1)
        self.recipient = parts[0]
        self.message = parts[1]


class BotServer(object):
    def __init__(self, host, bot_port, client_port):
        self.bots = []
        self.client = None
        self.__threads = []

        self.bot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.bot_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.bot_socket.bind((host, bot_port))
        self.bot_socket.listen(1)
        
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.client_socket.bind((host, client_port))
        self.client_socket.listen(1)
        
    def start(self):
        print('Running')
        while (True):
            ready_list, _, _ = select.select([self.bot_socket, self.client_socket], [], [])
            print(ready_list)
            if self.bot_socket in ready_list:
                self.on_bot_accept()
            if self.client_socket in ready_list:
                self.on_client_accept()
            
        
    def on_bot_accept(self):
        bot_connection, bot_address = self.bot_socket.accept()
        print('Accepting bot from {0}'.format(bot_address))
        b = Bot(bot_connection, self)
        self.bots.append(b)
        t = threading.Thread(target=b.loop)
        t.start()
        self.__threads.append(t)
        
    def on_client_accept(self):
        client_connection, client_address = self.client_socket.accept()
        if self.client == None:
            print('Accepting client from {0}'.format(client_address))
            c = Client(client_connection, self)
            self.client = c
            t = threading.Thread(target=c.loop)
            t.start()
            self.__threads.append(t)
        else:
            print('Client already connected!')
            client_connection.close()
            
    def on_bot_message(self, message):
        if self.client != None:
            self.client.send(message)
            
    def on_client_message(self, message):
        if '|' in message:
            m = Message(message)
            bot_list = [x for x in self.bots if x.bot_id == m.recipient]
            if len(bot_list) > 0:
                bot_list[0].send(m.message)
        else:
            for bot in self.bots:
                self.client.send(bot.bot_id)
        

class Bot(object):
    def __init__(self, socket, server):
        self.__socket = socket
        self.__buffer = ''
        self.__running = False

        self.handshake_completed = False
        self.bot_id = None
        self.server = server
        
    def send(self, data):
        self.__socket.sendall(data + '\r\n')
           
    def recv(self, buffer_size=1024):
        while '\r\n' not in self.__buffer:
            self.__buffer = self.__buffer + self.__socket.recv(buffer_size)
        
        while '\r\n' in self.__buffer:
            i = self.__buffer.find('\r\n')
            line = self.__buffer[:i]
            self.__buffer = self.__buffer[i + 2:]
            if self.handshake_completed:
                print('{0} > {1}'.format(self.bot_id, line))
                self.server.on_bot_message('{0}|{1}'.format(self.bot_id, line))
                # send message back to client
            else:
                self.handshake_completed = True
                self.bot_id = line
                print('Bot registered: {0}'.format(self.bot_id))
        
    def loop(self):
        self.__running = True
        while self.__running:
            time.sleep(0.5)
            self.recv()
        
        
class Client(object):
    def __init__(self, socket, server):
        self.__socket = socket
        self.__buffer = ''
        self.__running = False
        self.server = server
        
    def send(self, data):
        self.__socket.sendall(data + '\r\n')
           
    def recv(self, buffer_size=1024):
        while '\r\n' not in self.__buffer:
            self.__buffer = self.__buffer + self.__socket.recv(buffer_size)
        
        while '\r\n' in self.__buffer:
            i = self.__buffer.find('\r\n')
            line = self.__buffer[:i]
            self.__buffer = self.__buffer[i + 2:]
            print('CLIENT > {0}'.format(line))
            self.server.on_client_message(line)
        
    def loop(self):
        self.__running = True
        while self.__running:
            time.sleep(0.5)
            self.recv()
        

if __name__ == '__main__':
    server = BotServer('localhost', 8888, 9999)
    server.start()
    