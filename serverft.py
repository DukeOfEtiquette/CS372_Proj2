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

    # Listen with a backlog of 5
    sock.listen(1)

    while True:

        print "Waiting for incoming connection..."

        # Accept a connect and store sock and addr
        (conn, addr) = sock.accept()

        print 'Connection made with client, waiting for message...'

        # Receive the message and split on spaces to create a list
        msg = conn.recv(512)

        # REMOVE THIS BEFORE SUBMITTING
        print msg

        msg = msg.split(" ")

        print msg

        # If this is a listing request...
        if msg[0] == '-l':

            # Connect to data socket on client
            dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            print sock.getpeername()
            dataSock.connect((addr, msg[1]))

            # Search directly for all txt files
            files = glob.glob("*.txt")
            # Send all messages as a single string with space delim
            dataSock.send(" ".join(files))
            # Send a random, (likely) unique string to signal end of packet
            dataSock.send(" #$%")
            dataSock.close()

        # Close the connection
        conn.close()

except Exception as err:
    print err

















