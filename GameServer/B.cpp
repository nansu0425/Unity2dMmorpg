#include "GameServer/Pch.h"
#include "B.h"
#include "C.h"

void B::Write()
{
    WRITE_GUARD;
    // Do something
}

void B::WriteC(C& c)
{
    WRITE_GUARD;
    c.Write();
}
