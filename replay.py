def extract_raw_payloads(pcap_path, target_port):
    try:
        packets = rdpcap(pcap_path)
    except FileNotFoundError:
        print(f"[-] Error: '{pcap_path}' not found.")
        sys.exit(1)
        
    raw_payloads = []
    for packet in packets:
        if packet.haslayer(IP) and packet.haslayer(TCP):
            # REMOVED port filtering: extracts any TCP packet that has a payload
            if packet[TCP].payload:
                raw_payloads.append(bytes(packet[TCP].payload))
                
    return raw_payloads

if __name__ == "__main__":
    replay_raw_traffic()
