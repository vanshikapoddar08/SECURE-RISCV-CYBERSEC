import socket

# If Node B is on a different machine, replace 127.0.0.1 with Node B's real IP address
TARGET_IP = "127.0.0.1" 
TARGET_PORT = 4446

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((TARGET_IP, TARGET_PORT))
    print("[*] Connected! Injecting manual text packet...")
    
    # Send an explicit test string followed by a newline character
    s.sendall(b"--- REPLAY SIMULATION TEST STRING ---\n")
    s.close()
    print("[+] Sent successfully.")
except Exception as e:
    print(f"[-] Test failed: {e}")
