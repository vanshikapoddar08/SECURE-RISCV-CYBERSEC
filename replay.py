import socket
import sys
from scapy.all import rdpcap, TCP, IP

# === HARDCODED CONFIGURATION ===
PCAP_FILE = "traffic.pcap"
TARGET_IP = "192.168.1.50"  # <--- CHANGE THIS to Node B's actual IP address
TARGET_PORT = 4446          # Configured socat port
PROTOCOL = "TCP"

def extract_raw_payloads(pcap_path, target_port):
    """Extracts raw byte sequences sent to port 4446 from the PCAP."""
    try:
        packets = rdpcap(pcap_path)
    except FileNotFoundError:
        print(f"[-] Error: '{pcap_path}' not found in this directory.")
        sys.exit(1)
        
    raw_payloads = []
    for packet in packets:
        if packet.haslayer(IP) and packet.haslayer(TCP):
            # Isolate traffic sent to Node B's port that contains data
            if packet[TCP].dport == target_port and packet[TCP].payload:
                raw_payloads.append(bytes(packet[TCP].payload))
                
    return raw_payloads

def replay_raw_traffic():
    payloads = extract_raw_payloads(PCAP_FILE, TARGET_PORT)
    
    if not payloads:
        print(f"[-] No raw data payloads found heading to TCP port {TARGET_PORT} inside the PCAP.")
        return
        
    print(f"[+] Found {len(payloads)} payload segments to replay.")
    print(f"[*] Attempting connection to Node B ({TARGET_IP}:{TARGET_PORT})...")

    try:
        # Establish raw TCP handshakes to Node B's socat listener
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((TARGET_IP, TARGET_PORT))
        print(f"[+] Connected successfully.")
        
        # Inject the packets sequentially
        for idx, payload in enumerate(payloads):
            print(f"[>] Sending segment {idx+1}/{len(payloads)}: {payload}")
            client_socket.sendall(payload)
            
        client_socket.close()
        print("[+] Replay finished. Connection closed.")
    except Exception as e:
        print(f"[-] Connection failed: {e}")
        print("[-] Ensure Node B's socat listener is actively running and your TARGET_IP is correct.")

if __name__ == "__main__":
    replay_raw_traffic()
