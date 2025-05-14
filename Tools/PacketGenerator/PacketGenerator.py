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
    pkt_h_temp = env.get_template("Packet.h")
    pkt_cpp_temp = env.get_template("Packet.cpp")
    pkt_handler_h_temp = env.get_template("PacketHandler.h")

    # key: project_name, value: prefix_key
    project_to_prefix = {args.client_project_name: "S2C", args.server_project_name: "C2S"}

    # 각 프로젝트에 대해 패킷 코드를 생성한다
    for project_name, prefix_key in project_to_prefix.items():
        # 1. Pachket.h 렌더링
        pkt_h_render = pkt_h_temp.render(
            parser=payload_parser,
            project_name=project_name)

        # 2. Pachket.h 출력
        pkt_h_out_dir = os.path.join(
            solution_path, project_name, "Network", "Protocol")
        os.makedirs(pkt_h_out_dir, exist_ok=True)
        pkt_h_out = os.path.join(pkt_h_out_dir, "Packet.h")
        with open(pkt_h_out, 'w', encoding="utf-8") as f:
            f.write(pkt_h_render)

        # 3. Packet.cpp 렌더링
        pkt_cpp_render = pkt_cpp_temp.render(
            project_name=project_name)

        # 4. Packet.cpp 출력
        pkt_cpp_out_dir = os.path.join(
            solution_path, project_name, "Network", "Protocol")
        os.makedirs(pkt_cpp_out_dir, exist_ok=True)
        pkt_cpp_out = os.path.join(pkt_cpp_out_dir, "Packet.cpp")
        with open(pkt_cpp_out, 'w', encoding="utf-8") as f:
            f.write(pkt_cpp_render)

        # 5. PacketHandler.h 렌더링
        pkt_h_handler_render = pkt_handler_h_temp.render(
            parser=payload_parser,
            project_name=project_name,
            prefix_key=prefix_key)

        # 6. PacketHandler.h 출력
        pkt_handler_h_out_dir = os.path.join(
            solution_path, project_name, "Network")
        os.makedirs(pkt_handler_h_out_dir, exist_ok=True)
        pkt_handler_h_out = os.path.join(
            pkt_handler_h_out_dir, "PacketHandler.h")
        with open(pkt_handler_h_out, 'w', encoding="utf-8") as f:
            f.write(pkt_h_handler_render)

    return

if __name__ == "__main__":
    main()
