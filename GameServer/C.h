#pragma once

class A;

class C
{
public:
    void Write();
    void WriteA(A& a);

private:
    RW_SPIN_LOCK;
};
