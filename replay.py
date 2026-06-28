from scapy.all import rdpcap, IP, TCP, Raw, send

# Load the file into a list-like structure
pkts = rdpcap("traffic.pcap")
# Extract packet index 4 (the 5th packet in the file)
target_packet = pkts[0]
# 1. Update IP destination to local loopback
if target_packet.haslayer(IP):
    target_packet[IP].dst = "127.0.0.1"
    del target_packet[IP].chksum  # Delete old checksum so Scapy recalculates it

# 2. Update TCP destination port to match QEMU (4445)
if target_packet.haslayer(TCP):
    target_packet[TCP].dport = 4445
    del target_packet[TCP].chksum  # Delete old checksum so Scapy recalculates it
# Send the packet onto the network layer
send(target_packet)
print("Packet replayed successfully.")

