#include "GameServer/Pch.h"
#include "C.h"
#include "A.h"

void C::Write()
{
    WRITE_GUARD;
    // Do something
}

void C::WriteA(A& a)
{
    WRITE_GUARD;
    a.Write();
}
