#pragma once
#include <istream>
#include <ostream>
#include <fstream>

using namespace std;

class BitInputStream
{
private:
    istream &in;
    char buf;
    int nbits;

public:
    BitInputStream(string fileName);
    int readBit();
    int readInt();
    short readShort();
};

class BitOutputStream
{
private:
    ofstream &out;
    char buf;
    int nbits;

public:
    BitOutputStream(string fileName);
    ~BitOutputStream();
    void writeBit(int i);
    void writeShort(short i);
    void writeInt(int i);
    void writeCode(string code);
    void flush();
};