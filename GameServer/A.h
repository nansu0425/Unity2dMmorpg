#pragma once

class B;
class C;

class A
{
public:
    void Write();
    void WriteB(B& b);
    void RecursiveWrite(C& c);

private:
    RW_LOCK;
};
