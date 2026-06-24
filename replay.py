import socket
import threading

LISTEN_PORT = 5556

captured_packet = None

def forward(src, dst):
    global captured_packet

    while True:
        data = src.recv(1024)

        if not data:
            break

        print("Captured:", data.hex())

        captured_packet = data

        dst.sendall(data)

def replay(dst):
    global captured_packet

    while True:
        cmd = input("Type replay: ")

        if cmd == "replay":

            if captured_packet:

                print("Replaying packet...")

                dst.sendall(captured_packet)

server = socket.socket()
server.bind(("127.0.0.1", LISTEN_PORT))
server.listen(1)

print("Waiting for Node A...")

nodeA = server.accept()[0]

print("Node A connected")

nodeB = socket.socket()
nodeB.connect(("127.0.0.1", 4445))

print("Connected to Node B")

threading.Thread(
    target=forward,
    args=(nodeA,nodeB),
    daemon=True
).start()

replay(nodeB)
