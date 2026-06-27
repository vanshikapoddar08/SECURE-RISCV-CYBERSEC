import socket
import sys
from scapy.all import rdpcap, TCP, IP

# === CONFIGURATION ===
PCAP_FILE = "traffic.pcap"
TARGET_IP = "127.0.0.1"    # <--- Change this to Node B's real IP if needed
TARGET_PORT = 4446         # Your socat listener port
PROTOCOL = "TCP"

def extract_raw_payloads(pcap_path, target_port):
    """Extracts any raw TCP byte sequence that contains a data payload."""
    try:
        packets = rdpcap(pcap_path)
    except FileNotFoundError:
        print(f"[-] Error: '{pcap_path}' not found in this directory.")
        sys.exit(1)
        
    raw_payloads = []
    for packet in packets:
        if packet.haslayer(IP) and packet.haslayer(TCP):
            # Extract data from any TCP packet that contains content
            if packet[TCP].payload:
                raw_payloads.append(bytes(packet[TCP].payload))
                
    return raw_payloads

def replay_raw_traffic():
    """Connects to the target socket and replays the data segments."""
    payloads = extract_raw_payloads(PCAP_FILE, TARGET_PORT)
    
    if not payloads:
        print(f"[-] No raw data payloads found inside the PCAP file.")
        return
        
    print(f"[+] Found {len(payloads)} payload segments to replay.")
    print(f"[*] Attempting connection to Node B ({TARGET_IP}:{TARGET_PORT})...")

    try:
        # Establish connection to your socat listener
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((TARGET_IP, TARGET_PORT))
        print(f"[+] Connected successfully.")
        
        # Inject packets sequentially
        for idx, payload in enumerate(payloads):
            print(f"[>] Sending segment {idx+1}/{len(payloads)}: {payload}")
            client_socket.sendall(payload)
            
        client_socket.close()
        print("[+] Replay finished. Connection closed.")
    except Exception as e:
        print(f"[-] Connection failed: {e}")
        print("[-] Verify that Node B's socat listener is running and accessible.")

if __name__ == "__main__":
    replay_raw_traffic()
