from scapy.all import rdpcap, Raw
import socket

packets = rdpcap("traffic.pcap")

for packet in packets:

    if packet.haslayer(Raw):

        payload = packet[Raw].load
        print("Payload:", payload)

        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect(("127.0.0.1", 4446))
        client.send(payload)
        client.close()

        print("Replay completed.")
        break     
