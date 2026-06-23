import socket
import time

HOST = "127.0.0.1"
PORT = 4444  # Change if your display_front uses another port

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    print(f"Connecting to {HOST}:{PORT}...")
    sock.connect((HOST, PORT))

    message = b"HELLO\n"

    print("Sending original message...")
    sock.sendall(message)

    time.sleep(5)

    print("Replaying same message...")
    sock.sendall(message)

    print("Replay completed.")

    sock.close()

except Exception as e:
    print("Error:", e)
