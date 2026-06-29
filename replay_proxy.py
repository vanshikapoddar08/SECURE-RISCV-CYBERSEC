import socket
import threading
import sys

# -----------------------------
# Configuration
# -----------------------------
PROXY_HOST = "0.0.0.0"
PROXY_PORT = 4445          # Node A connects here

NODE_B_HOST = "127.0.0.1"
NODE_B_PORT = 4446         # Node B listens here

BUFFER_SIZE = 1024

captured_messages = []
lock = threading.Lock()


# -----------------------------
# Forward A --> B
# -----------------------------
def forward_a_to_b(node_a, node_b):
    global captured_messages

    while True:
        try:
            data = node_a.recv(BUFFER_SIZE)

            if not data:
                print("\nNode A disconnected.")
                break

            print("\n[A --> B]")
            print("Bytes :", data)
            print("Hex   :", data.hex())

            # Save a copy for replay
            with lock:
                captured_messages.append(data)

            node_b.sendall(data)

        except Exception as e:
            print("A->B Error:", e)
            break


# -----------------------------
# Forward B --> A
# -----------------------------
def forward_b_to_a(node_b, node_a):

    while True:
        try:
            data = node_b.recv(BUFFER_SIZE)

            if not data:
                print("\nNode B disconnected.")
                break

            print("\n[B --> A]")
            print("Bytes :", data)
            print("Hex   :", data.hex())

            node_a.sendall(data)

        except Exception as e:
            print("B->A Error:", e)
            break


# -----------------------------
# Replay Console
# -----------------------------
def replay_console(node_b):

    while True:

        print("\n------------------------------------")
        print("Commands:")
        print("r  -> Replay last message")
        print("l  -> List captured messages")
        print("q  -> Quit")
        print("------------------------------------")

        cmd = input("Command: ").strip().lower()

        if cmd == "r":

            with lock:

                if len(captured_messages) == 0:
                    print("No messages captured yet.")
                    continue

                msg = captured_messages[-1]

            print("\nReplaying...")
            print("Bytes :", msg)
            print("Hex   :", msg.hex())

            try:
                node_b.sendall(msg)
                print("Replay sent successfully.")

            except Exception as e:
                print("Replay failed:", e)

        elif cmd == "l":

            with lock:

                if len(captured_messages) == 0:
                    print("No captured messages.")
                    continue

                print("\nCaptured Messages:")

                for i, m in enumerate(captured_messages):
                    print(f"{i}: {m}    HEX={m.hex()}")

        elif cmd == "q":

            print("Exiting...")
            node_b.close()
            sys.exit(0)

        else:
            print("Unknown command.")


# -----------------------------
# Main
# -----------------------------
def main():

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    server.bind((PROXY_HOST, PROXY_PORT))

    server.listen(1)

    print("===================================")
    print(" Replay Proxy Started")
    print(" Waiting for Node A...")
    print(" Listening on port", PROXY_PORT)
    print("===================================")

    node_a, addr = server.accept()

    print("\nNode A Connected:", addr)

    node_b = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    node_b.connect((NODE_B_HOST, NODE_B_PORT))

    print("Connected to Node B.")

    thread1 = threading.Thread(
        target=forward_a_to_b,
        args=(node_a, node_b),
        daemon=True
    )

    thread2 = threading.Thread(
        target=forward_b_to_a,
        args=(node_b, node_a),
        daemon=True
    )

    thread1.start()
    thread2.start()

    replay_console(node_b)


if __name__ == "__main__":
    main()
