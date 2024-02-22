#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
using namespace std;

typedef char int8;
typedef pair<int,int> pairInt;

class DataLoader
{
private:
    
    string filePath;
    int numLines;
    int elementPerLine; // number of elements per line
    unsigned int numBytes;
    unsigned long long totalNumElements;
    vector<int> freqMap_2s;
    vector<int> freqMap_sm;
    // index: 0~15 -> stands for -8~7
    vector<pairInt> extremumMap_2s;
    vector<pairInt> extremumMap_sm;
    // index: 0~15 -> stands for -8~7 <Most occured times, Min occured times>
    vector<vector<short>> dataAry_freq_2s;
    vector<vector<short>> dataAry_freq_sm;
    bool is2s;
    // After initialization, it is 2s.
    // index: 0~numLines-1 -> stands for each line data frequency in 4 bits
    vector<vector<int8>> dataAry;
    // actual data in 8 bits
public:
    
    DataLoader();
    DataLoader(string filePath);
    void displayFreqMap();
    void displayExtremumMap();
    vector<vector<short>>& getDataAry_freq_2s();
    vector<vector<short>>& getDataAry_freq_sm();
    // transfer dataAry_freq from 2s to sm
    vector<int> getFreqMap_2s();
    vector<int> getFreqMap_2s(const set<int>& data_lines);
    // get freqMap_2s from data_lines
    vector<int> getFreqMap_sm();
    vector<int> getFreqMap_sm(const set<int>& data_lines);
    vector<pairInt> getExtremumMap_2s();
    vector<pairInt> getExtremumMap_2s(const set<int>& data_lines);
    vector<pairInt> getExtremumMap_sm();
    vector<pairInt> getExtremumMap_sm(const set<int>& data_lines);
    int getWidth();
    int getNumLines();
    int getelementPerLine();
    const vector<vector<int8>> &getDataAry();
    unsigned long long getdataSize();
};
