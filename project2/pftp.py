#!/usr/bin/env python
import re
import argparse
import socket
import sys, os
from threading import Thread 
from threading import Lock
import time
def logging(args, logfile, msg, thread_id = -1):
  if args.logfile:
    lock = Lock()
    with lock:
      if logfile:
        logfile.write(("Thread " + str(thread_id) + ": " if thread_id >= 0 else "") + "S->C: " + msg + "\n")
      else:
        print(("Thread " + str(thread_id) + ": " if thread_id >= 0 else "") + "S->C: " + msg + "\n", end = "")
      
def send_and_log(sock, args, logfile, msg, thread_id = -1):
  sock.send(msg.encode())
  if args.logfile:
    lock = Lock()
    with lock:
      if logfile:
        logfile.write(("Thread " + str(thread_id) + ": " if thread_id >= 0 else "") + "C->S: " + msg)
      else:
        print(("Thread " + str(thread_id) + ": " if thread_id >= 0 else "") + "C->S: " + msg, end = "")

def single_threaded_main(args):
  ## log file creation ##
  log = None
  if args.logfile != "-" and args.logfile != None:
    log = open(args.logfile, "w+")
  ## control connection ##
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  try:
    if log:
      log.write("C->S: Connect to Server: " + args.hostname + "\n")
    elif args.logfile == "-":
      print("C->S: Connect to Server: " + args.hostname + "\n", end = "")
    else:
      pass
    s.connect((args.hostname, args.port))
    
  except:
    s.close()
    sys.stderr.write("Can't connect to server: " + args.hostname + "\n")
    exit(1)
  portno = None
  while True:
    recv = s.recv(128).decode().strip()
    logging(args, log, recv)
    code = int(recv[0:3])
    if code == 220: # enter username
      send_and_log(s, args, log, ("USER " + args.user + "\n"))
    elif code == 331: # enter password
      send_and_log(s, args, log, ("PASS " + args.password + "\n"))
    elif code == 230: # login successfully -> switch to binary mode
      send_and_log(s, args, log, ("TYPE I\n")) #binary mode
    elif code == 200:  
      send_and_log(s, args, log, "PASV\n")
    elif code == 227: # entering passive mode -> get port number
      try:
        return_info = list(eval(recv.split()[-1][:-1])) # has period sign 
      except:
        return_info = list(eval(recv.split()[-1])) # no period sign
      portno = return_info[-1] + return_info[-2] * 256
      break
    ## error messages ##
    elif code == 530: # Login failed
      sys.stderr.write("Authentication failed\n")
      s.close()
      exit(2)
    elif code == 500: # Unkonwn command
      sys.stderr.write("Command not implemented by server\n")
      s.close()
      exit(5)
    elif code == 503: # Operation not allowed by server
      sys.stderr.write("Operation not allowed by server\n")
      s.close()
      exit(6)
    else:
      sys.stderr.write("Generic Error\n")
      s.close()
      exit(7)
  ## data connection ##
  if portno: 
    data_link = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_link.connect((args.hostname, portno))
    local_file = open(args.file, "bw+")
    # starting to receive
    send_and_log(s, args, log, ("RETR " + args.file + "\n"))
    recv = s.recv(128).decode()[:-1]
    logging(args, log, recv)
    code = int(recv[0:3])
    if code == 550:
      sys.stderr.write("File not found\n")
      data_link.close()
      s.close()
      os.remove(args.file)
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
      code = int(recv[0:3])
      if code == 226:
        send_and_log(s, args, log, "QUIT\n")
      if code == 221:
        s.close()
        break  

def parse_config_line(config_line):
  filename = config_line.split("/")[-1]
  hostname = config_line.split("/")[2].split("@")[-1]
  username = config_line.split("/")[2].split("@")[0].split(":")[0]
  password = config_line.split("/")[2].split("@")[0].split(":")[1]
  return filename, hostname, username, password

def multi_threaded_worker(args, config_lines, thread_id, block_size, log_file, num_server, file_size):
  filename, hostname, username, password = parse_config_line(config_lines[thread_id])
  offset = block_size * thread_id
  if thread_id == num_server - 1:
    block_size = file_size - block_size * (num_server - 1)
  local_file = open(filename, "bw+")
  local_file.seek(offset)
  ## control connection ##
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  try:
    s.connect((hostname, args.port))
    if log_file:
      log_file.write("Thread " + str(thread_id) + ": C->S: Connect to Server: " + hostname + "\n")
    elif args.logfile == "-":
      print("Thread " + str(thread_id) + ": C->S: Connect to Server: " + hostname + "\n", end = "")
  except:
    s.close()
    sys.stderr.write("Thread " + str(thread_id) + ": Can't connect to server\n")
    exit(1)
  portno = None
  while True:
    recv = s.recv(128).decode()[:-1]
    logging(args, log, recv, thread_id)
    code = int(recv[0:3])
    if code == 220: # enter username
      send_and_log(s, args, log, ("USER " + username + "\n"), thread_id)
    elif code == 331: # enter password
      send_and_log(s, args, log, ("PASS " + password + "\n"), thread_id)
    elif code == 230: # login successfully
      send_and_log(s, args, log, ("TYPE I\n"), thread_id) #binary mode
    elif code == 200: 
      send_and_log(s, args, log, "PASV\n", thread_id)
    elif code == 227: # entering passive mode
      try:
        return_info = list(eval(recv.split()[-1][:-1])) # has period sign
      except:
        return_info = list(eval(recv.split()[-1])) # no period sign
      portno = return_info[-1] + return_info[-2] * 256
      send_and_log(s, args, log, ("REST " + str(offset) + "\n"), thread_id)
    elif code == 350:
      break
    ## error messages ##
    elif code == 530: # Login failed
      sys.stderr.write("Thread " + str(thread_id) + ": Authentication failed\n")
      s.close()
      exit(2)
    elif code == 500: # Unkonwn command
      sys.stderr.write("Thread " + str(thread_id) + ": Command not implemented by server\n")
      s.close()
      exit(5)
    elif code == 503: # Operation not allowed by server
      sys.stderr.write("Thread " + str(thread_id) + ": Operation not allowed by server\n")
      s.close()
      exit(6)
    else:
      sys.stderr.write("Thread " + str(thread_id) + ": Generic Error\n")
      s.close()
      exit(7)
  ## data connection ##
  if portno: 
    data_link = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_link.connect((hostname, portno))
    # starting to receive
    send_and_log(s, args, log, ("RETR " + filename + "\n"), thread_id)
    recv = s.recv(128).decode()[:-1]
    logging(args, log, recv, thread_id)
    code = int(recv[0:3])
    if code == 550:
      sys.stderr.write("Thread " + str(thread_id) + " :File not found\n")
      data_link.close()
      s.close()
      exit(3)
    # receiving data
    recv_len = 0
    while True:
      recv_data = data_link.recv(512)
      recv_len += len(recv_data)
      local_file.write(recv_data)
      if not recv_data:
          break
      if recv_len > block_size:
        break
    
    data_link.close()
    # data transferring completed
    # communicating with server and close socket
    while True:
      recv = s.recv(128).decode()[:-1]
      logging(args, log, recv, thread_id)
      code = int(recv[0:3])
      if code == 426 or code == 226:
        send_and_log(s, args, log, "QUIT\n", thread_id)
      if code == 221:
        s.close()
        break  
  local_file.close()

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
  optional_group.add_argument("-l", "--log", help = "logs all the FTP commands exchanged with the server and the corresponding replies to file LOGFILE.", dest = "logfile")
  optional_group.add_argument("-t", "--thread", help = "get the config file, and run multi-threaded version", dest = "config")
  # parsing arguments
  args = parser.parse_args()
  
  # main process
  if args.version:
    print("FTP, version 0.1, Deqing Fu")
    exit(0)
  
  if (args.file == None or args.hostname == None) and args.config == None:
    sys.stderr.write("Syntax error in the client request\n")
    exit(4)

  ## Main ##
  if args.config: # Multithreading 
    #creating logfile
    log = None
    if args.logfile != "-" and args.logfile != None:
      log = open(args.logfile, "w+")

    # Parsing Config File
    N = 0 # Number of severs 
    lines = []
    with open(args.config) as config_file:
      while True:
        line = config_file.readline().strip()
        if not line: 
          break
        lines.append(line)
        N += 1
    ## First control connection ## Asking for size of the file
    filename, hostname, username, password = parse_config_line(lines[0])
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
      if log:
        log.write("C->S: Connect to Server: " + hostname + "\n")
      elif args.logfile == "-":
        print("C->S: Connect to Server: " + hostname + "\n", end = "")
      else:
        pass
      s.connect((hostname, args.port))
    except:
      s.close()
      sys.stderr.write("Can't connect to server\n")
      exit(1)
    file_size = 0
    while True:
      recv = s.recv(128).decode()[:-1]
      logging(args, log, recv)
      code = int(recv[0:3])
      if code == 220: # enter username
        send_and_log(s, args, log, ("USER " + username + "\n"))
      elif code == 331: # enter password
        send_and_log(s, args, log, ("PASS " + password + "\n"))
      elif code == 230: # login successfully -> ask for size
        send_and_log(s, args, log, ("SIZE " + filename + "\n"))
      elif code == 213: # get file size successfully
        file_size = int(recv.split()[1])
        s.close()
        break
    
    save = open(filename, "bw+")
    save.seek(file_size-1)
    save.write(b"\0")
    save.close()
    thread_list =[]
    block_size = file_size // N
    for i in range(N):
      t = Thread(target=multi_threaded_worker, args=(args, lines, i, block_size, log, N, file_size))
      if log:
        log.write("Starting Thread: " + str(i) + "\n")
      elif args.logfile == "-":
        print("Starting Thread: " + str(i) + "\n", end = "")
      thread_list.append(t)
      t.start()
    
    for i in range(N):
      t = thread_list[i]
      t.join()
      if log:
        log.write("Joining Thread: " + str(i) + "\n")
      elif args.logfile == "-":
        print("Joining Thread: " + str(i) + "\n", end = "")
      
    if log:
      log.close()
    # Verifying that the size of receiving file matches the size from server
    received_size = os.path.getsize(filename)
    if received_size < file_size:
      os.remove(filename)
      sys.stderr.write("Generic Error: File Received is incomplete\n")
      exit(7)

  else: # Single Threading
    single_threaded_main(args)