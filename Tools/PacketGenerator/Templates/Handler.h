/*    {{ project_name }}/Network/Handler.h    */

#pragma once

#include "Protocol/Packet/Dispatcher.h"

namespace {{ project_namespace }}
{
    class {{ proto_file }}_PacketDispatcher
        : public proto::PacketDispatcher
    {
    public:
        static {{ proto_file }}_PacketDispatcher& GetInstance()
        {
            static {{ proto_file }}_PacketDispatcher sInstance;
            return sInstance;
        }

    protected:  // 모든 패킷 핸들러 등록
                        {{ proto_file }}_PacketDispatcher() { RegisterAllHandlers(); }

        virtual void    RegisterAllHandlers() override
        {
            using namespace proto;
            {% for packet in proto_parser.packet_dict[proto_file] %}
            RegisterHandler<{{ packet.payload_type }}>(&Handle_{{ packet.payload_type }}, PacketId::{{ packet.payload_type }});
            {%- endfor %}
        }

    private:    // 모든 페이로드 핸들러
        {%- for packet in proto_parser.packet_dict[proto_file] %}
        static Bool     Handle_{{ packet.payload_type }}(SharedPtr<core::Session> owner, const proto::{{ packet.payload_type }}& payload);
        {%- endfor %}
    };
} // namespace {{ project_namespace }}
