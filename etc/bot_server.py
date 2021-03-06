import socket
import select
import threading
import time
import ssl
from datetime import datetime
import json

'''
TODO
- error on duplicate bot ID
- graceful closing
'''

PASSWORD = 'PASSWORD_LOL'

class Message(object):
    def __init__(self, message):
        parts = message.split('|', 1)
        self.recipient = parts[0]
        self.message = parts[1]


class BotServer(object):
    def __init__(self, host, bot_port, client_port, client_password):
        self.__client_password = client_password
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
            ready_list, _, _ = select.select([self.bot_socket, self.client_socket], [], [], 3)
            #print(ready_list)
            if self.bot_socket in ready_list:
                self.on_bot_accept()
            if self.client_socket in ready_list:
                self.on_client_accept()
            
        
    def on_bot_accept(self):
        bot_connection, bot_address = self.bot_socket.accept()
        stream = ssl.wrap_socket(bot_connection, 
                                 server_side=True,
                                 certfile='cert.pem',
                                 keyfile='key.pem',
                                 ssl_version=ssl.PROTOCOL_TLSv1_2)
        print('Accepting bot from {0}'.format(bot_address))
        b = Bot(stream, self)
        self.bots.append(b)
        t = threading.Thread(target=b.loop)
        t.daemon = True
        t.start()
        self.__threads.append(t)
        
    def on_client_accept(self):
        client_connection, client_address = self.client_socket.accept()
        stream = ssl.wrap_socket(client_connection, 
                                 server_side=True,
                                 certfile='cert.pem',
                                 keyfile='key.pem',
                                 ssl_version=ssl.PROTOCOL_TLSv1_2)
        if self.client == None:
            print('Accepting client from {0}'.format(client_address))
            c = Client(stream, self, self.__client_password)
            self.client = c
            t = threading.Thread(target=c.loop)
            t.daemon = True
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
            self.handle_server_message(message)

    def handle_server_message(self, message):
        if message == 'bot_list':
            full_list = ';'.join(map(lambda x: x.bot_id, self.bots))
            self.client.send('[bot_list]|' + full_list)
        elif message.startswith('info_'):
            bot_id = message.split('_')[1]
            bot_list = [x for x in self.bots if x.bot_id == bot_id]
            if len(bot_list) > 0:
                bot_data = bot_list[0].get_info()
                self.client.send(message + '|' + json.dumps(bot_data))
        else:
            self.client.send('[-]|unknown command: ' + message)

    def bot_joined(self, bot):
        print('Bot registered: {0}'.format(bot.bot_id))
        if self.client != None:
            self.client.send('[bot_joined]|' + bot.bot_id)
        

    def bot_leaving(self, bot):
        self.bots.remove(bot)
        print('Bot leaving: ' + bot.bot_id)
        if self.client != None:
            self.client.send('[bot_left]|' + bot.bot_id)
        # what about thread?
        
    def client_leaving(self):
        self.client = None
        print('Client leaving')
        

class Bot(object):
    def __init__(self, socket, server):
        self.__socket = socket
        self.__buffer = ''
        self.__running = False

        self.handshake_completed = False
        self.bot_id = None
        self.registered_time = None
        self.server = server
        
    def send(self, data):
        print('{0} -> {1}'.format(self.bot_id, data))
        self.__socket.sendall((data + '\r\n').encode('utf-8'))
           
    def recv(self, buffer_size=1024):
        while '\r\n' not in self.__buffer:
            try:
                data = self.__socket.recv(buffer_size)
            except ConnectionError:
                return False
            if len(data) == 0:
                return False
            self.__buffer = self.__buffer + data.decode('utf-8')
        
        while '\r\n' in self.__buffer:
            i = self.__buffer.find('\r\n')
            line = self.__buffer[:i]
            self.__buffer = self.__buffer[i + 2:]
            if self.handshake_completed:
                print('{0} > {1}'.format(self.bot_id, line))
                self.server.on_bot_message('{0}|{1}'.format(self.bot_id, line))
                # send message back to client
            else:
                if line in map(lambda x: x.bot_id, self.server.bots):
                    # Dupe
                    self.send('DUPE')
                else:
                    self.handshake_completed = True
                    self.bot_id = line
                    self.registered_time = datetime.utcnow()
                    self.send('OK')
                    self.server.bot_joined(self)

        return True

        
    def loop(self):
        self.__running = True
        while self.__running:
            time.sleep(0.1)
            if not self.recv():
                break;
        self.__running = False
        self.server.bot_leaving(self)

    def get_info(self):
        return {
            'addr': str(self.__socket.getpeername()),
            'reg': str(self.registered_time)
        }

        
class Client(object):
    def __init__(self, socket, server, client_password):
        self.__socket = socket
        self.__client_password = client_password
        self.__buffer = ''
        self.__running = False
        self.server = server
        self.handshake_completed = False
        
    def send(self, data):
        print('CLIENT < {0}'.format(data))
        self.__socket.sendall((data + '\r\n').encode('utf-8'))
        
    def recv(self, buffer_size=1024):
        while '\r\n' not in self.__buffer:
            data = self.__socket.recv(buffer_size)
            if len(data) == 0:
                return False
            self.__buffer = self.__buffer + data.decode('utf-8')
        
        while '\r\n' in self.__buffer:
            i = self.__buffer.find('\r\n')
            line = self.__buffer[:i]
            self.__buffer = self.__buffer[i + 2:]
            if self.handshake_completed:
                print('CLIENT > {0}'.format(line))
                self.server.on_client_message(line)
            else:
                if line == self.__client_password:
                    self.handshake_completed = True
                    print('CLIENT auth completed')
                    self.send('OK')
                else:
                    print('CLIENT auth failed')
                    self.send('AUTH_FAILED')
            
        return True
        
    def loop(self):
        self.__running = True
        while self.__running:
            time.sleep(0.1)
            if not self.recv():
                break
        self.server.client_leaving()
        

if __name__ == '__main__':
    f = open('server_config.json')
    config = json.load(f)
    f.close()
    server = BotServer(config['server_host'], config['bot_port'], config['client_port'], config['client_password'])
    server.start()
    
