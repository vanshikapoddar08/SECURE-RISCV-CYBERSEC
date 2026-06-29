import socket
import threading

# ---------------- Configuration ----------------
LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 4445          # Node A connects here

NODE_B_HOST = "127.0.0.1"
NODE_B_PORT = 4446          # Node B server

BUFFER_SIZE = 1024

captured_packets = []
node_b_socket = None
# ------------------------------------------------


def forward_a_to_b(node_a, node_b):
    global captured_packets

    while True:
        data = node_a.recv(BUFFER_SIZE)

        if not data:
            print("Node A disconnected")
            break

        print(f"\n[A -> B] {data}")

        # Save packet for replay
        captured_packets.append(data)

        # Forward to Node B
        node_b.sendall(data)


def forward_b_to_a(node_b, node_a):

    while True:
        data = node_b.recv(BUFFER_SIZE)

        if not data:
            print("Node B disconnected")
            break

        print(f"\n[B -> A] {data}")

        node_a.sendall(data)


def replay_console():
    global node_b_socket

    while True:

        cmd = input("\nPress r to replay: ")

        if cmd.lower() != "r":
            continue

        if len(captured_packets) == 0:
            print("No packet captured yet.")
            continue

        payload = captured_packets[-1]

        print("\n===== REPLAY =====")
        print(payload)
        print("==================")

        node_b_socket.sendall(payload)

        print("Replay sent!")


# ---------------- MAIN ----------------

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

server.bind((LISTEN_HOST, LISTEN_PORT))
server.listen(1)

print(f"Waiting for Node A on port {LISTEN_PORT}...")

node_a_socket, addr = server.accept()

print("Node A connected:", addr)

node_b_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

print("Connecting to Node B...")
node_b_socket.connect((NODE_B_HOST, NODE_B_PORT))

print("Connected to Node B.")

threading.Thread(
    target=forward_a_to_b,
    args=(node_a_socket, node_b_socket),
    daemon=True
).start()

threading.Thread(
    target=forward_b_to_a,
    args=(node_b_socket, node_a_socket),
    daemon=True
).start()

replay_console()
    
