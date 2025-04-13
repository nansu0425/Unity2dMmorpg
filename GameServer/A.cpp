#include "GameServer/Pch.h"
#include "A.h"
#include "B.h"

void A::Write()
{
    WRITE_GUARD;
    // Do something
}

void A::WriteB(B& b)
{
    WRITE_GUARD;
    b.Write();
}
