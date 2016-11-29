#/user/bin/python

import os.path
import sys
import socket
import signal
import glob

# make sure usage is correct
if len(sys.argv) != 2:
    print "usage: ./code.py port"
    sys.exit(1)


# Register signal and define signal function
def signal_handler(signal, frame):

    # CLOSE IT
    sock.close()

    print "Exiting with the highest amount of grace."
    sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)

# User localhost and port# passed at cmd line
host = 'flip2'
port = sys.argv[1]

print host
print port

try:
    # Open socket and bind to desired host and port
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

        # Receive the message and split on % to create a list
        msg = conn.recv(512)
        msg = msg.split("%")

        # If this is a listing request...
        if msg[0] == '-l':

            # Connect to data socket on client
            dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            dataSock.connect((addr[0], int(float(msg[1]))))
            print "Data socket connected"

            # Search directly for all txt files and space separate them
            files = glob.glob("*.txt")
            files = " ".join(files)

            # Send all messages as a single string with space delim
            dataSock.send(files)

            # Close socket now that we are done sending
            print "Finished sending message"
            dataSock.close()
        elif msg[0] == '-g':

            # Connect to data socket on client
            dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            dataSock.connect((addr[0], int(float(msg[2]))))
            print "Data socket connected"

            if os.path.isfile(msg[1]):
                with open(msg[1], 'r') as f:
                    for line in f:
                        print "Sending ->", line
                        dataSock.send(line)
                #f = open(msg[1], 'r')
                #content = f.read()
                #print "Sending -> ", content
                #dataSock.send(content)
            else:
                dataSock.send("{0} does not exist on server".format(msg[1]))

            dataSock.close()
        else:

            # Connect to data socket on client
            dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            dataSock.connect((addr[0], int(float(msg[1]))))
            print "Data socket connected"

            # Send message across data socket that an unknown arg was sent
            dataSock.send("Bad arguement sent, unable to complete request.")

        # Close the connection
        conn.close()

except Exception as err:
    print err















