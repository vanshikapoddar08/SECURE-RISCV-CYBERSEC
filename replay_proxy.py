import socket
import threading

LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 4445

NODEB_HOST = "127.0.0.1"
NODEB_PORT = 4446

BUFFER = 1024

captured = []

# --------------------------
# Listen for Node A
# --------------------------

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

server.bind((LISTEN_HOST, LISTEN_PORT))
server.listen(1)

print("Waiting for Node A...")

nodeA, addr = server.accept()

print("Node A Connected!")

# --------------------------
# Connect to Node B
# --------------------------

nodeB = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

print("Connecting to Node B...")

nodeB.connect((NODEB_HOST, NODEB_PORT))

print("Connected to Node B!")

# --------------------------
# Forward A -> B
# --------------------------

def forward_A_to_B():

    while True:

        data = nodeA.recv(BUFFER)

        if not data:
            break

        print("\nCaptured Bytes:")
        print(data)
        print(data.hex())

        captured.append(data)

        nodeB.sendall(data)

# --------------------------
# Forward B -> A
# --------------------------

def forward_B_to_A():

    while True:

        data = nodeB.recv(1024)

        if not data:
            print("Node B disconnected")
            break

        print("\nFROM NODE B")
        print(data)
        print(data.hex())

        nodeA.sendall(data)
# --------------------------
# Replay Menu
# --------------------------

while True:

    print("\n")
    print("r  -> Replay last packet")
    print("l  -> List packets")
    print("q  -> Quit")

    cmd = input("> ")

    if cmd == "l":

        print("\nCaptured Packets\n")

        for i, pkt in enumerate(captured):

            print(i, pkt.hex())

    elif cmd == "r":

        if len(captured) == 0:

            print("Nothing captured.")

            continue

        pkt = captured[-1]

        print("\nReplaying...")

        nodeB.sendall(pkt)

        print("Replay Completed.")

    elif cmd == "q":
        break
