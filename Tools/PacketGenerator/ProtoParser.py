#    Tools/PacketGenerator/ProtoParser.py    #

import re
import os

class ProtoParser:
    def __init__(self, start_packet_id):
        self.packet_dict = {}  # key: proto 파일 이름, value: Packet 객체 리스트
        self.start_packet_id = start_packet_id
        self.next_packet_id = start_packet_id

    def parse_proto_files(self, proto_files):
        for file_path in proto_files:
            # 파일 이름만 추출 (확장자 제외)
            file_name = os.path.basename(file_path)
            file_name_without_ext = os.path.splitext(file_name)[0]
            
            # 해당 파일에 대한 패킷 리스트 초기화
            self.packet_dict[file_name_without_ext] = []
            
            # 파일의 모든 라인을 읽는다
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # message 정의 모두 찾기
            message_pattern = r'message\s+(\w+)'
            messages = re.findall(message_pattern, content)
            
            # 파일 내의 모든 메시지를 Packet으로 변환하여 추가
            for msg_name in messages:
                packet = Packet(msg_name, self.next_packet_id)
                self.packet_dict[file_name_without_ext].append(packet)
                self.next_packet_id += 1  # packet_id 증가
    
    def get_packet_dict(self):
        return self.packet_dict

class Packet:
    def __init__(self, payload_type, packet_id):
        self.payload_type = payload_type
        self.packet_id = packet_id
