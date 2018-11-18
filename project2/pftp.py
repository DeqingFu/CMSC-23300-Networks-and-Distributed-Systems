import re
import argparse
import socket
import sys, os
from threading import Thread 

def main(args):
  # control connection
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  try:
    print(args.hostname, args.port)
    s.connect((args.hostname, args.port))
    print("connection succeeded")
  except:
    s.close()
    sys.exit("connection failed")
  portno = None
  while True:
    recv = s.recv(128).decode()
    print(recv, end = "")
    code = int(recv[0:3])
    #print(code)
    if code == 220: # enter username
      s.send(("USER " + args.user + "\n").encode())
    elif code == 331: # enter password
      s.send(("PASS " + args.password + "\n").encode())
      #print(args.password)
    elif code == 230: # login successfully
      s.send(("TYPE I\n").encode()) #binary mode
    elif code == 200: 
      s.send("PASV\n".encode())
    elif code == 227: # entering passive mode
      return_info = list(eval(recv.split()[-1][:-1]))
      portno = return_info[-1] + return_info[-2] * 256
      #print(portno)
      break
  # data connection
  if portno: 
    print("receiving file" + args.file)
    data_link = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_link.connect((args.hostname, portno))
    local_file = open(args.file, "bw+")
    s.send(("RETR " + args.file + "\n").encode())
    while True:
      recv_data = data_link.recv(1024)
      if not recv_data:
        break
      else:
        local_file.write(recv_data)
    data_link.close()
  s.close()
       
if __name__ == "__main__":
  if len(sys.argv) == 1:
    try:
      os.system("python pftp.py -h")
    except:
      os.system("python3 pftp.py -h")
    exit(0)
    
  parser = argparse.ArgumentParser()
  # commands
  parser.add_argument("-v", "--version", help = "the version number, the author", action = "store_true")

  # required group
  required_group = parser.add_argument_group("Require Arguments")
  required_group.add_argument("-f", "--file", help = "the file to download")
  required_group.add_argument("-s", "--server", help = "the server to download the file from", dest = "hostname")

  # options group
  optional_group = parser.add_argument_group("Options")
  optional_group.add_argument("-p", "--port", help = "the prot to be used when contacting the server", default = 21, type = int, dest = "port")
  optional_group.add_argument("-n", "--username", help = "uses the username when logging into the FTP server", default = "anonymous", dest = "user")
  optional_group.add_argument("-P", "--password", help = "uses the password when logging into the FTP server", default = "user@localhost.localnet")
  optional_group.add_argument("-m", "--mode", help = "the mode to be used for the transfer", default = "binary")
  optional_group.add_argument("-l", "--log", help = "Logs all the FTP commands exchanged with the server and the corresponding replies to file LOGFILE.", dest = "logfile")

  # parsing arguments
  args = parser.parse_args()
  
  # main process
  if args.version:
    print("FTP, version 0.1, Deqing Fu")
    sys.exit()
  
  if args.file == None or args.hostname == None:
    sys.exit("Requiring File name and Server name")

  if args.logfile:
    log = open(args.logfile, "w+")
  
  main(args)
