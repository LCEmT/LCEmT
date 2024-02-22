#pragma once

#include <iostream>
#include <vector>
const int ARRAY_SIZE = 32;
using namespace std;

class TreeArray
{
private:
    vector<short> array;
    //– [#0-bit codes, #1-bit codes, #2-bit codes, …]

    int numNode;
    short numLeaf;
    short maxCodeLength;
    short minCodeLength;
    static vector<vector<TreeArray>> legalTreeMemoryTable;
    // index = 0, stands for legal tree with 1 leaf
    // index = 1, stands for legal tree with 2 leaf
    // index = 2, stands for legal tree with 3 leaf ......

    static vector<TreeArray> getlegalTrees(int numLeaf);
public:
    TreeArray(); //default tree 1 node
    TreeArray(int numLeaf);
    TreeArray(vector<short> array);
    int getMaxCodeLength();
    int getMinCodeLength();
    bool checkKraft();
    vector<short> getCodeArray();
    static vector<TreeArray> getlegalTreesDP(int numLeaf);
    static TreeArray mergeTree(TreeArray& leftTree, TreeArray& rightTree);
    friend ostream& operator<<(ostream& os, const TreeArray& tree);
};
