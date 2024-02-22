#pragma once
#include "CodeTree.h"
#include "BitStream.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
typedef char int8;


class CodeTree;

/*
 * Reads from a Huffman-coded bit stream and decodes symbols.
 */
class Decoder final
{

private:
    BitInputStream in;
    ofstream out;
    bool is2s;
    bool is2s_2;
    bool twoTree;
    const CodeTree *codeTree;
    const CodeTree *codeTree_2;
    int intPerLine;
    int numLines;
    // helper for generating CodeTree
    CodeTree *CanonicalToCodeTree(vector<int> codeLengths);
    int decodeOne(int &line_width, bool useSecondTree);
public:
    explicit Decoder(string encodedFileName, string decodedFileName,bool twoTree);
    void decode();
    int max_line_width;
};