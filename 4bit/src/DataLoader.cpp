#include "DataLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

int getMaxIndex(const vector<short>& freqMap){
    int maxIndex = 0;
    for(int i=0;i<freqMap.size();i++){
        if(freqMap[i] > freqMap[maxIndex]){
            maxIndex = i;
        }
    }
    return maxIndex;
}

int getMinIndex(const vector<short>& freqMap){
    int minIndex = 0;
    for(int i=0;i<freqMap.size();i++){
        if(freqMap[i] < freqMap[minIndex]){
            minIndex = i;
        }
    }
    return minIndex;
}

DataLoader::DataLoader() : numLines(0), elementPerLine(128)
{
    extremumMap_2s = vector<pairInt>(16, make_pair(0,INT32_MAX));
    freqMap_2s = vector<int>(16, 0);
    extremumMap_sm = vector<pairInt>(16, make_pair(0,INT32_MAX));
    freqMap_sm = vector<int>(16, 0);
    totalNumElements = 0;
    is2s = true;
}

DataLoader::DataLoader(string filePath) : filePath(""), numLines(0)
{
    this->filePath = filePath;
    totalNumElements = 0;
    freqMap_2s = vector<int>(16, 0);
    extremumMap_2s = vector<pairInt>(16, make_pair(0,INT32_MAX));
    freqMap_sm = vector<int>(16, 0);
    extremumMap_sm = vector<pairInt>(16, make_pair(0,INT32_MAX));
    is2s = true;

    // read data using mmap
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd == -1)
    {
        cout << "Error: can't open file" << endl;
        exit(1);
    }
    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        cout << "Error: can't get file size" << endl;
        exit(1);
    }
    size_t length = sb.st_size;
    int8 *addr = (int8 *)mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
    {
        cout << "Error: mmap failed" << endl;
        exit(1);
    }
    elementPerLine = *(int *)addr;
    numLines = *((int *)addr + 1);
    dataAry = vector<vector<int8>>(numLines, vector<int8>(elementPerLine, 0));

    totalNumElements = elementPerLine * numLines * 2;

    int8 *cur = (int8 *)(addr + 2 * sizeof(int8));
    int num = 0;
    // cout<<"elementPerLine: "<<elementPerLine<<endl;
    // cout<<"numLines: "<<numLines<<endl;
    // cout<<"totalNumElements: "<<totalNumElements<<endl;
    for (int i = 0; i < numLines; i++)
    {
        vector<short> tempFreqMap_2s(16, 0);
        vector<short> tempFreqMap_sm(16, 0);
        for (int j = 0; j < elementPerLine; j++)
        {
            num = *(cur++);
            dataAry[i][j] = num;
            int8 s_M = ((num + (num >> 7)) ^ (num >> 7)) | (num & 0x80);
            int8 upper4bits_2s = (num >> 4) & 0x0F;
            int8 lower4bits_2s = num & 0x0F;
            int8 upper4bits_sm = (s_M >> 4) & 0x0F;
            int8 lower4bits_sm = s_M & 0x0F;
            freqMap_2s[upper4bits_2s]++;
            freqMap_2s[lower4bits_2s]++;
            freqMap_sm[upper4bits_sm]++;
            freqMap_sm[lower4bits_sm]++;
            tempFreqMap_2s[upper4bits_2s]++;
            tempFreqMap_2s[lower4bits_2s]++;
            tempFreqMap_sm[upper4bits_sm]++;
            tempFreqMap_sm[lower4bits_sm]++;
        }
        dataAry_freq_2s.push_back(tempFreqMap_2s);
        dataAry_freq_sm.push_back(tempFreqMap_sm);
        int maxIndex = getMaxIndex(tempFreqMap_2s);
        int minIndex = getMinIndex(tempFreqMap_2s);
        for(int i=0;i<extremumMap_2s.size();i++){
            if(tempFreqMap_2s[i] > extremumMap_2s[i].first){
                extremumMap_2s[i].first = tempFreqMap_2s[i];
            }
            if(tempFreqMap_2s[i] < extremumMap_2s[i].second){
                extremumMap_2s[i].second = tempFreqMap_2s[i];
            }
        }
        maxIndex = getMaxIndex(tempFreqMap_sm);
        minIndex = getMinIndex(tempFreqMap_sm);
        for(int i=0;i<extremumMap_sm.size();i++){
            if(tempFreqMap_sm[i] > extremumMap_sm[i].first){
                extremumMap_sm[i].first = tempFreqMap_sm[i];
            }
            if(tempFreqMap_sm[i] < extremumMap_sm[i].second){
                extremumMap_sm[i].second = tempFreqMap_sm[i];
            }
        }
    }
    munmap(addr, length);
    close(fd);
    numBytes = numLines * elementPerLine;
}

void DataLoader::displayFreqMap(){
    cout<<"FreqMap in 2's complement: "<<endl;
    for(int i=0;i<freqMap_2s.size();i++){
        cout<<(int)(i-8)<<": "<<freqMap_2s[i]<<endl;
    }
    cout<<"FreqMap in sign-magnitude: "<<endl;
    for(int i=0;i<freqMap_sm.size();i++){
        cout<<(int)(i-8)<<": "<<freqMap_sm[i]<<endl;
    }
}

void DataLoader::displayExtremumMap(){
    cout<<"ExtremumMap in 2's complement: "<<endl;
    for(int i=0;i<extremumMap_2s.size();i++){
        cout<<(int)(i-8)<<": "<<extremumMap_2s[i].first<<" "<<extremumMap_2s[i].second<<endl;
    }
    cout<<"ExtremumMap in sign-magnitude: "<<endl;
    for(int i=0;i<extremumMap_sm.size();i++){
        cout<<(int)(i-8)<<": "<<extremumMap_sm[i].first<<" "<<extremumMap_sm[i].second<<endl;
    }
}

vector<vector<short>>& DataLoader::getDataAry_freq_2s(){
    return dataAry_freq_2s;
}

vector<vector<short>>& DataLoader::getDataAry_freq_sm(){
    return dataAry_freq_sm;
}

vector<int> DataLoader::getFreqMap_2s(){
    return freqMap_2s;
}

vector<int> DataLoader::getFreqMap_2s(const set<int>& data_lines){
    vector<int> freqMap_2s(16, 0);
    for(int n: data_lines){
        for(int j=0;j<16;j++){
            freqMap_2s[j] += dataAry_freq_2s[n][j];
        }
    }
    return freqMap_2s;
}

vector<pairInt> DataLoader::getExtremumMap_2s(){
    return extremumMap_2s;
}

vector<pairInt> DataLoader::getExtremumMap_2s(const set<int>& data_lines){
    vector<pairInt> extremumMap_2s(16, make_pair(0,INT32_MAX));
    for(int n: data_lines){
        vector<short> tempFreqMap_2s(16, 0);
        for(int j=0;j<dataAry[n].size();j++){
            int8 num = dataAry[n][j];
            int8 upper4bits_2s = (num >> 4) & 0x0F;
            int8 lower4bits_2s = num & 0x0F;
            tempFreqMap_2s[upper4bits_2s]++;
            tempFreqMap_2s[lower4bits_2s]++;
        }
        for(int j=0;j<extremumMap_2s.size();j++){
            if(tempFreqMap_2s[j] > extremumMap_2s[j].first){
                extremumMap_2s[j].first = tempFreqMap_2s[j];
            }
            if(tempFreqMap_2s[j] < extremumMap_2s[j].second){
                extremumMap_2s[j].second = tempFreqMap_2s[j];
            }
        }
    }
    return extremumMap_2s;
}

vector<int> DataLoader::getFreqMap_sm(){
    return freqMap_sm;
}

vector<int> DataLoader::getFreqMap_sm(const set<int>& data_lines){
    vector<int> freqMap_sm(16, 0);
    for(int n: data_lines){
        for(int j=0;j<16;j++){
            freqMap_sm[j] += dataAry_freq_sm[n][j];
        }
    }
    return freqMap_sm;
}

vector<pairInt> DataLoader::getExtremumMap_sm(){
    return extremumMap_sm;
}

vector<pairInt> DataLoader::getExtremumMap_sm(const set<int>& data_lines){
    vector<pairInt> extremumMap_sm(16, make_pair(0,INT32_MAX));
    for(int n: data_lines){
        vector<short> tempFreqMap_sm(16, 0);
        for(int j=0;j<dataAry[n].size();j++){
            int8 num = dataAry[n][j];
            int8 s_M = ((num + (num >> 7)) ^ (num >> 7)) | (num & 0x80);
            int8 upper4bits_sm = (s_M >> 4) & 0x0F;
            int8 lower4bits_sm = s_M & 0x0F;
            tempFreqMap_sm[upper4bits_sm]++;
            tempFreqMap_sm[lower4bits_sm]++;
        }
        for(int j=0;j<extremumMap_sm.size();j++){
            if(tempFreqMap_sm[j] > extremumMap_sm[j].first){
                extremumMap_sm[j].first = tempFreqMap_sm[j];
            }
            if(tempFreqMap_sm[j] < extremumMap_sm[j].second){
                extremumMap_sm[j].second = tempFreqMap_sm[j];
            }
        }
    }
    return extremumMap_sm;
}

int DataLoader::getWidth(){
    return elementPerLine*8;
}

int DataLoader::getNumLines(){
    return numLines;
}

int DataLoader::getelementPerLine(){
    return elementPerLine;
}

const vector<vector<int8>> &DataLoader::getDataAry()
{
    return dataAry;
}

unsigned long long DataLoader::getdataSize(){
    return numBytes;
}
