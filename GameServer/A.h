#pragma once

class B;

class A
{
public:
    void Write();
    void WriteB(B& b);

private:
    RW_SPIN_LOCK;
};
