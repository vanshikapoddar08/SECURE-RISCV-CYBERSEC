from scapy.all import rdpcap, Raw
import socket

# 1. Load the capture file
pkts = rdpcap("traffic.pcap")

# 2. Grab the first packet
target_packet = pkts[0]

# 3. Safely pull out only the message data payload
if target_packet.haslayer(Raw):
    msg_bytes = target_packet[Raw].load
    print(f"Extracted payload data: {msg_bytes}")

    # 4. Open a real OS connection to QEMU so it legally accepts the data
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(("127.0.0.1", 4446)) # Connects to Node B
        s.sendall(msg_bytes)           # Injects the actual message bytes
        s.close()
        print("Message successfully pushed into Node B's serial line!")
    except ConnectionRefusedError:
        print("Error: QEMU is not running or not listening on port 4446.")
else:
    print("The first packet does not contain a raw data payload.")
