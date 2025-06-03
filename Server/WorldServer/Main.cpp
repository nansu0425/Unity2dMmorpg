/*    WorldServer/Main.cpp    */

#include "WorldServer/Pch.h"
#include "Network/Pch.h"
#include "Protocol/Pch.h"
#include "GameLogic/Pch.h"

int main()
{
    core::HelloWorld();
    net::HelloWorld();
    proto::HelloWorld();
    game::HelloWorld();

    return 0;
}
