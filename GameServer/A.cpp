#include "GameServer/Pch.h"
#include "A.h"
#include "B.h"
#include "C.h"

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

void A::RecursiveWrite(C& c)
{
    WRITE_GUARD;
    c.WriteA(*this);
}
