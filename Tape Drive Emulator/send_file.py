# arduino file uploader (11.02.13)
# by eric barch (ericbarch.com)

# imports
import socket, os, time, sys

# check for parameters
if len(sys.argv) != 3:
    print 'improper syntax, follow format:'
    print 'send_file.py 192.168.1.xxx cnc.txt'
    exit()

# the file we want to send
cncFile = sys.argv[2]
# number of bytes per packet
blocksize = 256
# ip address of arduino
ip = sys.argv[1]
# port of arduino
port = 23


# connection setup
conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
conn.connect((ip, port))
# notify the arduino we want to load a new file
conn.send('n')

# keep track of these so we know completion percent
fileSize = os.path.getsize(cncFile)
bytesSent = 0

# make sure the file exists
if os.path.exists(cncFile):
    # open the file
    with open(cncFile, 'rb') as f:
        # initial packet
        packet = f.read(blocksize)

        # send data while we have data to send
        while packet != '':
            # send the packet to the arduino
            conn.send(packet)

            # wait for a response from the arduino
            while True:
                # arduino sends a single confirmation byte
                msg = conn.recv(1)
                if len(msg) == 1:
                    # calculate and print completion percent
                    bytesSent += blocksize
                    percent = int((bytesSent/float(fileSize)) * 100)
                    if percent > 100:
                        percent = 100
                    print str(percent) + "% complete"
                    break
            
            # read in the next packet
            packet = f.read(blocksize)

        conn.close()