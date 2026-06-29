import socket
import threading

LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 4445

NODE_B_HOST = "127.0.0.1"
NODE_B_PORT = 4446

BUFFER_SIZE = 1024

captured_packets = []

node_a = None
node_b = None


def forward_a_to_b():
    while True:

        data = node_a.recv(BUFFER_SIZE)

        if not data:
            print("Node A disconnected")
            break

        print("\n[A -> B]")
        print(data)

        captured_packets.append(data)

        node_b.sendall(data)


def forward_b_to_a():
    while True:

        data = node_b.recv(BUFFER_SIZE)

        if not data:
            print("Node B disconnected")
            break

        print("\n[B -> A]")
        print(data)

        node_a.sendall(data)


server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

server.bind((LISTEN_HOST, LISTEN_PORT))

server.listen(1)

print("=================================")
print("Replay Proxy")
print("Listening on port", LISTEN_PORT)
print("Waiting for Node A...")
print("=================================")

node_a, addr = server.accept()

print("Node A connected:", addr)

node_b = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

print("Connecting to Node B...")

node_b.connect((NODE_B_HOST, NODE_B_PORT))

print("Connected to Node B!")

threading.Thread(target=forward_a_to_b, daemon=True).start()
threading.Thread(target=forward_b_to_a, daemon=True).start()


while True:

    print("\n-------------------------")
    print("Commands")
    print("l -> List packets")
    print("r -> Replay last")
    print("-------------------------")

    cmd = input("> ")

    if cmd == "l":

        if len(captured_packets) == 0:

            print("No packets.")

            continue

        for i, pkt in enumerate(captured_packets):

            print(i, pkt)

    elif cmd == "r":

        if len(captured_packets) == 0:

            print("Nothing captured.")

            continue

        pkt = captured_packets[-1]

        print("Replaying:", pkt)

        node_b.sendall(pkt)

        print("Replay complete.")
replay_console()
