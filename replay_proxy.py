import socket
import threading

LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 4445

NODEB_HOST = "127.0.0.1"
NODEB_PORT = 4446

BUFFER = 1024

captured = []

print("Connecting to Node B...")

# Connect to Node B FIRST (like socat)
while True:
    try:
        nodeB = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        nodeB.connect((NODEB_HOST, NODEB_PORT))
        print("Connected to Node B!")
        break
    except ConnectionRefusedError:
        pass

print("Listening for Node A...")

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((LISTEN_HOST, LISTEN_PORT))
server.listen(1)

nodeA, addr = server.accept()

print("Node A connected:", addr)


def A_to_B():

    while True:

        try:
            data = nodeA.recv(BUFFER)

            if not data:
                break

            print("\n[A -> B]")
            print(data)
            print("HEX:", data.hex())

            captured.append(data)

            nodeB.sendall(data)

        except Exception as e:
            print(e)
            break


def B_to_A():

    while True:

        try:
            data = nodeB.recv(BUFFER)

            if not data:
                break

            print("\n[B -> A]")
            print(data)

            nodeA.sendall(data)

        except Exception as e:
            print(e)
            break


threading.Thread(target=A_to_B, daemon=True).start()
threading.Thread(target=B_to_A, daemon=True).start()


while True:

    print("\n-------------------------")
    print("l : List captured packets")
    print("r : Replay last packet")
    print("-------------------------")

    cmd = input("> ")

    if cmd == "l":

        if len(captured) == 0:
            print("Nothing captured.")
            continue

        for i, pkt in enumerate(captured):
            print(i, pkt)

    elif cmd == "r":

        if len(captured) == 0:
            print("Nothing captured.")
            continue

        pkt = captured[-1]

        print("\nReplaying...")
        print(pkt)

        nodeB.sendall(pkt)

        print("Replay Done.")
