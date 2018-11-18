#!/usr/bin/env python
import re
import argparse
import socket
import sys, os
from threading import Thread 
def logging(args, logfile, msg):
  if args.logfile:
    if logfile:
      logfile.write("S->C: " + msg + "\n")
    else:
      print("S->C: " + msg)
      
def send_and_log(sock, args, logfile, msg):
  sock.send(msg.encode())
  if args.logfile:
    if logfile:
      logfile.write("C->S: " + msg)
    else:
      print("C->S: " + msg, end = "")

def main(args):
  # control connection
  log = None
  if args.logfile != "-" and args.logfile != None:
    log = open(args.logfile, "w+")

  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  try:
    #print(args.hostname, args.port)
    s.connect((args.hostname, args.port))
  except:
    s.close()
    print("Can't connect to server")
    exit(1)
  
  portno = None
  while True:
    recv = s.recv(128).decode()[:-1]
    logging(args, log, recv)
    code = int(recv[0:3])
    if code == 220: # enter username
      send_and_log(s, args, log, ("USER " + args.user + "\n"))
    elif code == 331: # enter password
      send_and_log(s, args, log, ("PASS " + args.password + "\n"))
    elif code == 230: # login successfully
      send_and_log(s, args, log, ("TYPE I\n")) #binary mode
    elif code == 200: 
      send_and_log(s, args, log, "PASV\n")
    elif code == 227: # entering passive mode
      return_info = list(eval(recv.split()[-1][:-1]))
      portno = return_info[-1] + return_info[-2] * 256
      break
    ## error messages ##
    elif code == 530: # Login failed
      print("Authentication failed")
      s.close()
      exit(2)
    elif code == 500: # Unkonwn command
      print("Command not implemented by server")
      s.close()
      exit(5)
    else:
      print("Generic Error")
      s.close()
      exit(7)
  
  # data connection
  if portno: 
    '''
    s.send(("SIZE " + args.file + "\n").encode())
    file_size = int(s.recv(128).decode()[4:-2])
    print(file_size)
    '''
    data_link = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_link.connect((args.hostname, portno))
    local_file = open(args.file, "bw+")
    # starting to receive
    send_and_log(s, args, log, ("RETR " + args.file + "\n"))
    #s.send(("RETR " + args.file + "\n").encode())
    recv = s.recv(128).decode()[:-1]
    logging(args, log, recv)
    code = int(recv[0:3])
    if code == 550:
      print("File not found")
      data_link.close()
      s.close()
      exit(3)
    # receiving data
    while True:
      recv_data = data_link.recv(1024)
      if not recv_data:
        break
      else:
        local_file.write(recv_data)
    data_link.close()
    # data transferring completed
    # communicating with server and close socket
    while True:
      recv = s.recv(128).decode()[:-1]
      logging(args, log, recv)
      #print(recv, end = "")
      code = int(recv[0:3])
      if code == 226:
        send_and_log(s, args, log, "QUIT\n")
      if code == 221:
        s.close()
        break  
       
if __name__ == "__main__":
  if len(sys.argv) == 1:
    os.system("./pftp.py -h")
    exit(0)
  ## Parsing ##
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
  #optional_group.add_argument("-m", "--mode", help = "the mode to be used for the transfer", default = "binary")
  optional_group.add_argument("-l", "--log", help = "Logs all the FTP commands exchanged with the server and the corresponding replies to file LOGFILE.", dest = "logfile")

  # parsing arguments
  args = parser.parse_args()
  
  # main process
  if args.version:
    print("FTP, version 0.1, Deqing Fu")
    exit(0)
  
  if args.file == None or args.hostname == None:
    print("Syntax error in the client request")
    exit(4)

  
  ## Main ##
  main(args)
