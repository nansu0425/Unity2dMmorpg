/*    {{ project_name }}/Network/PacketHandler.h    */

#pragma once

#include "{{ project_name }}/Network/Protocol/Packet.h"

/**
 * @class {{ prefix_key }}_PacketHandlerMap
 * @brief {{ prefix_key }} 패킷을 위한 특수 패킷 핸들러 맵입니다.
 * 
 * 각 특정 패킷 유형에 대한 핸들러를 등록하고 패킷 ID를 기반으로 
 * 적절한 핸들러에 패킷 처리를 위임합니다.
 */
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
