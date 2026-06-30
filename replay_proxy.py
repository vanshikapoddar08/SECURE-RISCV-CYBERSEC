import socket
import threading

# ----------------------------
# Configuration
# ----------------------------
LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 4445          # Node A connects here

NODE_B_HOST = "127.0.0.1"
NODE_B_PORT = 4446          # Node B server

last_packet = b''


# ----------------------------
# Forward Node A --> Node B
# ----------------------------
def forward_to_b(a_sock, b_sock):
    global last_packet

    while True:
        data = a_sock.recv(1024)

        if not data:
            break

        print("\nNode A --> Node B")
        print(data)

        # Save every packet
        last_packet = data

        # Forward immediately
        b_sock.sendall(data)


# ----------------------------
# Forward Node B --> Node A
# ----------------------------
def forward_to_a(a_sock, b_sock):

    while True:
        data = b_sock.recv(1024)

        if not data:
            break

        print("\nNode B --> Node A")
        print(data)

        # Forward ACK / reply
        a_sock.sendall(data)


# ----------------------------
# Main
# ----------------------------
print("Connecting to Node B...")

node_b = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
node_b.connect((NODE_B_HOST, NODE_B_PORT))

print("Connected to Node B")

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((LISTEN_HOST, LISTEN_PORT))
server.listen(1)

print("Waiting for Node A...")

node_a, addr = server.accept()

print("Node A Connected")

# Start forwarding threads
threading.Thread(target=forward_to_b,
                 args=(node_a, node_b),
                 daemon=True).start()

threading.Thread(target=forward_to_a,
                 args=(node_a, node_b),
                 daemon=True).start()

# ----------------------------
# Replay Menu
# ----------------------------
# ----------------------------
# Replay Menu (Fixed with Fresh Session Initialization)
# ----------------------------
while True:
    cmd = input("\nPress r to replay : ")
    if cmd == "r":
        if last_packet:
            print("Replaying packet over a new session...")
            try:
                # Create a completely separate socket connection to mimic a fresh client session
                replay_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                replay_sock.connect((NODE_B_HOST, NODE_B_PORT))
                
                # If Node B expects a handshake (SYN) first, you must handle or send it here
                
                # Send the captured packet
                replay_sock.sendall(last_packet)
                
                # Optional: Read response to see if Node B accepts it
                response = replay_sock.recv(1024)
                print(f"Node B Response to Replay: {response}")
                
                replay_sock.close()
            except Exception as e:
                print(f"Replay failed: {e}")
        else:
            print("No packet captured yet.")
