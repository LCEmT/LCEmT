#pragma once

#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>
#include "TreeArray.h"
#include "DataLoader.h"

using namespace std;

typedef char int8;

enum ASSIGN{ FQ, EX};
enum ENCODE{ TWOS, SM};
struct encodeInfo{
    ASSIGN a;
    ENCODE e;
    unsigned short w;
    encodeInfo(){};
    encodeInfo(ASSIGN a, ENCODE e, unsigned short w):a(a),e(e),w(w){}
};

class Encoder
{
private:
    int num_threads;
    // Data
    DataLoader *DL;
    vector<TreeArray> trees;

    // Table
    vector<short> global_best_table;
    vector<short> current_table;
    // Info for single tree method
    int global_best_width;
    encodeInfo global_best_encodeInfo;
    // Info for two trees method
    encodeInfo info1;
    encodeInfo info2;
    // parameter for tow trees method
    int TWOTABLELIMIT;
    int num_use_tree;
    // For Encoding
    map<int, string> genCanonCode(vector<short> table);
    // Functions
    int getMaxWidth_2s(vector<short> table, int storeIndex);
    int getMaxWidth_2s(vector<short> table, const set<int>& data_lines);
    int getMaxWidth_sm(vector<short> table, int storeIndex);
    int getMaxWidth_sm(vector<short> table, const set<int>& data_lines);
    int getWidth(vector<short> table, int data_line, bool is2s);
    vector<short> getTable(int index);
    int buildTable_FQ();
    encodeInfo buildTable_FQ(const set<int>& data_lines);
    int buildTable_EX();
    encodeInfo buildTable_EX(const set<int>& data_lines);
    int buildTable_FQ_SM();
    encodeInfo buildTable_FQ_SM(const set<int>& data_lines);
    int buildTable_EX_SM();
    encodeInfo buildTable_EX_SM(const set<int>& data_lines);
    void buildTwoTable_FQ(int& GlobalBestWidth, vector<short>& table1, vector<short>& table2, vector<bool>& group);
    void buildTwoTable_EX(int& GlobalBestWidth, vector<short>& table1, vector<short>& table2, vector<bool>& group);
    void buildTwoTable_FQ_SM(int& GlobalBestWidth, vector<short>& table1, vector<short>& table2, vector<bool>& group);
    void buildTwoTable_EX_SM(int& GlobalBestWidth, vector<short>& table1, vector<short>& table2, vector<bool>& group);
    encodeInfo buildLocalBestTable(const set<int>& data_lines, vector<short>& table);

public:
    Encoder(vector<TreeArray> trees, DataLoader *DL);
    // build table using a single tree
    int buildGlobalBestTable();
    // build table using two trees
    int buildTables(vector<short>& table1, vector<short>& table2, vector<bool>& group);
    // encode data using table made by a single tree
    void encode_oneT(string outputFileName);
    // encode data using tables made by two trees
    void encode_twoT(string outputFileName,vector<short> t1, vector<short>t2, vector<bool> group);
    vector<short> getTable();
    int max_encoding_length;
    ~Encoder();
};