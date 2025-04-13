#pragma once

class C;

class B
{
public:
    void Write();
    void WriteC(C& c);

private:
    RW_SPIN_LOCK;
};
