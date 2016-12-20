import BaseHTTPServer
import SocketServer
import cgi
import urlparse
from datetime import datetime

class CustomHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])

        u = urlparse.urlparse(self.path)
        qs = urlparse.parse_qs(u.query)
        client_id = qs['c'][0]

        print('client_id: {0}'.format(client_id))

        form = cgi.FieldStorage(
            fp=self.rfile,
            headers=self.headers,
            environ={'REQUEST_METHOD':'POST',
                     'CONTENT_TYPE':self.headers['Content-Type'],
                     })

        total_uploaded = 0
        for key in form.keys():
            if form[key].filename:
                original_filename = form[key].filename
                file_contents = form[key].file.read()
                print('\t' + original_filename)
                local_date = datetime.utcnow().strftime('%Y%m%d%H%M%S')
                local_filename = '{0}_{1}_{2}'.format(client_id, local_date, original_filename)
                f = open(local_filename, 'wb')
                f.write(file_contents)
                f.close()
                total_uploaded = total_uploaded + len(file_contents)

        self.send_response(200, str(total_uploaded))

if __name__ == '__main__':
    httpd = SocketServer.TCPServer(('', 8080), CustomHandler)
    httpd.serve_forever()

