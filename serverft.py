#/user/bin/python

import sys
import socket
import signal
import glob

#make sure usage is correct
if len(sys.argv) != 2:
    print "usage: ./code.py port"
    sys.exit(1)


#Register signal and define signal function
def signal_handler(signal, frame):

    #CLOSE IT
    sock.close()

    print "Exiting with the highest amount of grace."
    sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)

#User localhost and port# passed at cmd line
host = 'flip2'
port = sys.argv[1]

print host
print port

try:
    #Open socket and bind to desired host and port
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((socket.gethostbyname(host), int(port)))

    print 'Socket created.' 

    #Listen with a backlog of 5
    sock.listen(1)

    while True:

        print "Waiting for incoming connection..."

        #Accept a connect and store sock and addr
        (conn, addr) = sock.accept()

        print 'Connection made with client, waiting for message...'

        msg = conn.recv(512).split(" ")

        print msg

        if msg[0] == '-l':
            files = glob.glob("*.txt")
            for f in files:
                conn.send(f)

        conn.close()
    
except Exception as err:
    print err

















