#    Tools/PacketGenerator/PacketGenerator.py    #

import argparse
import jinja2
import os
import ProtoParser

def main():
    # Protocol 경로를 구한다
    script_path = os.path.dirname(os.path.abspath(__file__))
    tools_path = os.path.dirname(script_path)
    solution_path = os.path.dirname(tools_path)
    protocol_path = os.path.join(solution_path, "Common", "Protocol")

    # 인자 파싱
    arg_parser = argparse.ArgumentParser(description="packet generator arg parser")
    arg_parser.add_argument("--client_project_name", type=str, default="DummyClient", help="Client Project Name")
    arg_parser.add_argument("--server_project_name", type=str, default="GameServer", help="Server Project Name")
    args = arg_parser.parse_args()

    # Protocol 경로의 모든 payload 파일의 경로를 구한다
    payload_files = []
    for root, dirs, files in os.walk(protocol_path):
        for file in files:
            if file.endswith(".proto") and file != "Common.proto":
                payload_files.append(os.path.join(root, file))

    # 모든 payload 파일 파싱
    payload_parser = ProtoParser.ProtoParser(1000)
    payload_parser.parse_proto(payload_files)

    # jinja2 설정
    file_loader = jinja2.FileSystemLoader("Templates")
    env = jinja2.Environment(loader=file_loader)
    pkt_template = env.get_template("Packet.h")
    pkt_handler_template = env.get_template("PacketHandler.h")

    #----------------------------#
    # DummyClient의 패킷 코드 생성 #
    #----------------------------#

    # 1. Pachket.h 렌더링
    pkt_render = pkt_template.render(
        parser = payload_parser,
        project_name = args.client_project_name)

    # 2. Pachket.h 출력
    pkt_output_dir = os.path.join(solution_path, args.client_project_name, "Network", "Protocol")
    os.makedirs(pkt_output_dir, exist_ok=True)
    pkt_output = os.path.join(pkt_output_dir, "Packet.h")
    with open(pkt_output, 'w', encoding="utf-8") as f:
        f.write(pkt_render)

    # 3. PacketHandler.h 렌더링
    pkt_handler_render = pkt_handler_template.render(
        parser=payload_parser,
        project_name=args.client_project_name,
        prefix_key="S2C")

    # 4. PacketHandler.h 출력
    pkt_handler_output_dir = os.path.join(solution_path, args.client_project_name, "Network")
    os.makedirs(pkt_handler_output_dir, exist_ok=True)
    pkt_handler_output = os.path.join(pkt_handler_output_dir, "PacketHandler.h")
    with open(pkt_handler_output, 'w', encoding="utf-8") as f:
        f.write(pkt_handler_render)

    #---------------------------#
    # GameServer의 패킷 코드 생성 #
    #---------------------------#

    # 1. Pachket.h 렌더링
    pkt_render = pkt_template.render(
        parser=payload_parser,
        project_name=args.server_project_name)

    # 2. Pachket.h 출력
    pkt_output_dir = os.path.join(solution_path, args.server_project_name, "Network", "Protocol")
    os.makedirs(pkt_output_dir, exist_ok=True)
    pkt_output = os.path.join(pkt_output_dir, "Packet.h")
    with open(pkt_output, 'w', encoding="utf-8") as f:
        f.write(pkt_render)

    # 3. PacketHandler.h 렌더링
    pkt_handler_render = pkt_handler_template.render(
        parser=payload_parser,
        project_name=args.server_project_name,
        prefix_key="C2S")

    # 4. PacketHandler.h 출력
    pkt_handler_output_dir = os.path.join(solution_path, args.server_project_name, "Network")
    os.makedirs(pkt_handler_output_dir, exist_ok=True)
    pkt_handler_output = os.path.join(pkt_handler_output_dir, "PacketHandler.h")
    with open(pkt_handler_output, 'w', encoding="utf-8") as f:
        f.write(pkt_handler_render)

    return

if __name__ == "__main__":
    main()
