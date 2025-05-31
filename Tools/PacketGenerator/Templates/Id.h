/*    Protocol/Packet/Id.h    */

#pragma once
{% for proto_file, packets in proto_parser.packet_dict.items() %}
#include "Protocol/Payload/{{ proto_file }}.pb.h"
{%- endfor %}

namespace proto
{
    enum class PacketId : Int16
    {
        Invalid = 0,
        {%- for proto_file, packets in proto_parser.packet_dict.items() %}
        {%- for packet in packets %}
        {{ packet.payload_type }} = {{ packet.packet_id }},
        {%- endfor %}
        {%- endfor %}
    };
} // namespace proto
