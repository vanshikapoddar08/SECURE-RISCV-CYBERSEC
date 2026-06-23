import socket
import time

HOST = "127.0.0.1"
PORT = 4444

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

msg = "ATTACKTEST\n"

print("Sending original...")
sock.sendall(msg.encode())

time.sleep(3)

print("Replaying...")
sock.sendall(msg.encode())

sock.close()
