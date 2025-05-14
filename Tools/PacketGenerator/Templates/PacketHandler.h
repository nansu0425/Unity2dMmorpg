/*    {{ project_name }}/Network/PacketHandler.h    */

#pragma once

#include "{{ project_name }}/Network/Protocol/Packet.h"

class {{ prefix_key }}_PacketHandlerMap
    : public PacketHandlerMap
{
public:
    static {{ prefix_key }}_PacketHandlerMap& GetInstance()
    {
        static {{ prefix_key }}_PacketHandlerMap sInstance;
        return sInstance;
    }

protected:  // 모든 패킷 핸들러 등록
    {{ prefix_key }}_PacketHandlerMap() { RegisterAllHandlers(); }

    virtual void    RegisterAllHandlers() override
    {
        {% for payload in parser.payload_dict[prefix_key] %}
        RegisterHandler(
            [this](const Packet& packet)
            {
                return HandlePayload<{{ payload.name }}>({{ prefix_key }}_PacketHandlerMap::Handle_{{ payload.name }}, packet);
            },
            PacketId::{{ payload.name }});
        {% endfor %}
    }

private:    // 모든 페이로드 핸들러
    {%- for payload in parser.payload_dict[prefix_key] %}
    static Bool     Handle_{{ payload.name }}(SharedPtr<Session> owner, const {{ payload.name }}& payload);
    {%- endfor %}
};
