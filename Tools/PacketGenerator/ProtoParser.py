#    Tools/PacketGenerator/ProtoParser.py    #

import os

class ProtoParser:
    def __init__(self, start_id):
        self.payload_dict = {}
        self.start_id = start_id
        self.next_id = start_id

    def parse_proto(self, payload_files):
        for file in payload_files:
            # payload의 prefix를 구한다
            file_name = os.path.basename(file)
            payload_prefix = file_name.split(".")[0].split("_")[0]
            # prefix를 key로 하는 list를 생성한다
            self.payload_dict[payload_prefix] = []

            # payload 파일의 모든 라인을 읽는다
            with open(file, "r") as f:
                lines = f.readlines()

            for line in lines:
                line = line.strip()
                if line.startswith("message"):
                    msg_name = line.split(" ")[1]
                    # 메시지가 prefix로 시작하는지 확인한다
                    if not msg_name.startswith(payload_prefix):
                        continue

                    # payload type을 prefix에 해당하는 list에 추가한다
                    self.payload_dict[payload_prefix].append(Payload(msg_name, self.next_id))
                    self.next_id += 1

class Payload:
    def __init__(self, name, pkt_id):
        self.name = name
        self.pkt_id = pkt_id
