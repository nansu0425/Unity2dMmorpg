#    Tools/PacketGenerator/PacketGenerator.py    #

import jinja2
import os
import ProtoParser

def main():
    # Protocol 디렉토리를 구한다
    script_dir = os.path.dirname(os.path.abspath(__file__))
    tools_dir = os.path.dirname(script_dir)
    root_dir = os.path.dirname(tools_dir)
    proto_dir = os.path.join(root_dir, "Shared", "Protocol")

    # Protocol 디렉토리의 Common을 제외한 모든 proto 파일 이름을 구한다
    proto_files = []
    for root, dirs, files in os.walk(proto_dir):
        for file in files:
            if file.endswith(".proto") and file != "Common.proto":
                proto_files.append(os.path.join(root, file))

    # proto 파일들을 파싱하여 packet_dict를 만든다
    proto_parser = ProtoParser.ProtoParser(1000)
    proto_parser.parse_proto_files(proto_files)

    # jinja2 설정
    file_loader = jinja2.FileSystemLoader("Templates")
    env = jinja2.Environment(loader=file_loader)
    dispatcher_template = env.get_template("Dispatcher.h")
    packet_template = env.get_template("Packet.h")

    # Dispatcher.h 렌더링
    dispatcher_rendered = dispatcher_template.render(
        proto_parser=proto_parser)

    # Dispatcher.h 출력
    dispatcher_out_dir = os.path.join(root_dir, "Server", "Protocol", "Packet")
    os.makedirs(dispatcher_out_dir, exist_ok=True)
    dispatcher_out = os.path.join(dispatcher_out_dir, "Dispatcher.h")
    with open(dispatcher_out, 'w', encoding="utf-8") as f:
        f.write(dispatcher_rendered)

    # 패킷 코드를 생성할 project 관련 인자
    project_name_to_args = {
        "DummyClient": {"namespace": "dummy", "proto_file": "ToClient"},
        "WorldServer": {"namespace": "world", "proto_file": "ToWorld"}}

    for project_name, project_args in project_name_to_args.items():
        # Packet.h 렌더링
        packet_rendered = packet_template.render(
            proto_parser=proto_parser,
            project_name=project_name,
            project_namespace=project_args["namespace"],
            proto_file=project_args["proto_file"])

        # Pachket.h 출력
        packet_out_dir = os.path.join(root_dir, "Server", project_name, "Network")
        os.makedirs(packet_out_dir, exist_ok=True)
        packet_out = os.path.join(packet_out_dir, "Packet.h")
        with open(packet_out, 'w', encoding="utf-8") as f:
            f.write(packet_rendered)

    return

if __name__ == "__main__":
    main()
