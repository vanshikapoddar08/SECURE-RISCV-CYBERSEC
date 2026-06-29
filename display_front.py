import socket
import threading
import sys

# Connect to QEMU's UART0
QEMU_HOST = '127.0.0.1' 
QEMU_PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 4444

# Create a clean prompt name based on the port
clint_n = "Node A: " if QEMU_PORT == 4444 else "Node B: "

try:
    print(f"[*] Connecting to RISC-V on {QEMU_HOST}:{QEMU_PORT}...")
    qemu_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    qemu_socket.connect((QEMU_HOST, QEMU_PORT))
    print("[+] Connected! You can now type messages.")
    print("-" * 50)
except ConnectionRefusedError:
    print("[-] Connection failed. Is QEMU running?")
    sys.exit(1)

def listen_to_riscv():
    """Background thread that safely prints incoming messages without breaking the prompt"""
    buffer = ""
    while True:
        try:
            data = qemu_socket.recv(256)
            if not data:
                break
                
            text = data.decode('utf-8', errors='ignore')
            buffer += text
            
            # Only print when we receive a full message (ended by \n)
            if '\n' in buffer:
                lines = buffer.split('\n')
                buffer = lines.pop() # Keep any incomplete fragments in the buffer
                
                for line in lines:
                    # \r moves cursor to start of line, \033[K erases the current prompt
                    sys.stdout.write('\r\033[K')
                    
                    # Print the incoming message
                    sys.stdout.write(f"[Peer]: {line}\n")
                
                # Redraw the user's prompt so they can keep typing!
                sys.stdout.write(clint_n)
                sys.stdout.flush()
        except:
            break

# Start the listener thread
listener = threading.Thread(target=listen_to_riscv, daemon=True)
listener.start()

# Main loop: Take human input and send it to RISC-V UART0
try:
    while True:
        # Get input from the user
        message = input(clint_n)
        
        # Add the newline back so it travels across the C network!
        message = message + '\n'
        
        # Send it to the C program
        qemu_socket.sendall(message.encode())
except KeyboardInterrupt:
    print("\n[*] Closing connection.")
    qemu_socket.close()
    sys.exit(0)
