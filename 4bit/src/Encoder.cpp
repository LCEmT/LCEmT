#include "Encoder.h"
#include <algorithm>
#include <bitset>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime> 
#include <omp.h>
#include "Utils.h"
#include "BitStream.h"

class Info{
public:
    int value;
    int freq;
    int maxExtremum;
    int minExtremum;
    Info(int value, int freq, int maxExtremum, int minExtremum){
        this->value = value;
        this->freq = freq;
        this->maxExtremum = maxExtremum;
        this->minExtremum = minExtremum;
    }
};

Encoder::Encoder(vector<TreeArray> trees, DataLoader *DL)
{
    this->trees = trees;
    this->DL = DL;
    global_best_width = INT32_MAX;
    num_threads = 8;
    TWOTABLELIMIT = 10;
}

int Encoder::getWidth(vector<short> table, int data_line, bool is2s)
{
    int result = 0;
    if(is2s){
        for(int i=0;i<16;i++){
            result += DL->getDataAry_freq_2s()[data_line][i]*table[i];
        }
    }
    else{
        for(int i=0;i<16;i++){
            result += DL->getDataAry_freq_sm()[data_line][i]*table[i];
        }
    }
    return result;
}

int Encoder::getMaxWidth_2s(vector<short> table, int storeIndex=0)
{
    vector<unsigned short> result(DL->getNumLines(), 0);
    for(int i=0;i<DL->getNumLines();i++){
        int tmp = 0;
        for(int j=0;j<16;j++){
            tmp += DL->getDataAry_freq_2s()[i][j]*table[j];
        }
        result[i] = tmp;
    }
    return *max_element(result.begin(), result.end());
}

int Encoder::getMaxWidth_2s(vector<short> table, const set<int>& data_lines)
{
    vector<unsigned short> result;
    for(int n:data_lines){
        int tmp = 0;
        for(int j=0;j<16;j++){
            tmp += DL->getDataAry_freq_2s()[n][j]*table[j];
        }
        result.push_back(tmp);
    }
    return *max_element(result.begin(), result.end());
}

int Encoder::getMaxWidth_sm(vector<short> table, int storeIndex=0)
{
    vector<unsigned short> result(DL->getNumLines(), 0);
    for(int i=0;i<DL->getNumLines();i++){
        int tmp = 0;
        for(int j=0;j<16;j++){
            tmp += DL->getDataAry_freq_sm()[i][j]*table[j];
        }
        result[i] = tmp;
    }
    return *max_element(result.begin(), result.end());
}

int Encoder::getMaxWidth_sm(vector<short> table, const set<int>& data_lines)
{
    vector<unsigned short> result;
    for(int n:data_lines){
        int tmp = 0;
        for(int j=0;j<16;j++){
            tmp += DL->getDataAry_freq_sm()[n][j]*table[j];
        }
        result.push_back(tmp);
    }
    return *max_element(result.begin(), result.end());
}

int Encoder::buildTable_FQ()
{
    current_table.clear();
    vector<int> freqMap = DL->getFreqMap_2s();
    vector<pairInt> EXmap = DL->getExtremumMap_2s();
    //sort the freqMap based on the frequency
    vector<Info> valueInfoMap;
    // <frequency, number>
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.freq != b.freq)
            return a.freq > b.freq;
        if(a.maxExtremum!=b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    TreeArray BestTree;
    vector<short> BestTable(16, 0);
    int BestWidth = INT32_MAX;
    
    #pragma omp parallel for num_threads(num_threads)
    for(int i=0;i<trees.size();i++){
        
        vector<short> tmpTable(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        // [nums of 0 bits code, nums of 1 bits code, ...] sum = 16
        int codeLength = 0;
        // let higher frequency symbol have shorter code length
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            tmpTable[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        int currentWidth = getMaxWidth_2s(tmpTable ,i);
        
        #pragma omp critical
        {   
            if(currentWidth < BestWidth){
                BestWidth = currentWidth;
                BestTree = trees[i];
                BestTable = tmpTable;
            }
        }
    }
    current_table = BestTable;
    if(BestWidth < global_best_width){
        global_best_width = BestWidth;
        global_best_table = BestTable;
        global_best_encodeInfo = encodeInfo(FQ, TWOS, BestWidth);
    }
    return BestWidth;
}

encodeInfo Encoder::buildTable_FQ(const set<int>& data_lines){
    current_table.clear();
    vector<int> freqMap = DL->getFreqMap_2s(data_lines);
    vector<pairInt> EXmap = DL->getExtremumMap_2s(data_lines);
    //sort the freqMap based on the frequency
    vector<Info> valueInfoMap;
    // <frequency, number>
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.freq != b.freq)
            return a.freq > b.freq;
        if(a.maxExtremum!=b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    TreeArray BestTree;
    vector<short> BestTable(16, 0);
    int BestWidth = INT32_MAX;

    #pragma omp parallel for num_threads(num_threads)
    for(int i=0;i<trees.size();i++){
        vector<short> tmpTable(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        // [nums of 0 bits code, nums of 1 bits code, ...] sum = 16
        int codeLength = 0;
        // let higher frequency symbol have shorter code length
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            tmpTable[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        int currentWidth = getMaxWidth_2s(tmpTable,data_lines);
        
        #pragma omp critical
        {   
            if(currentWidth < BestWidth){
                BestWidth = currentWidth;
                BestTree = trees[i];
                BestTable = tmpTable;
            }
        }
    }
    current_table = BestTable;
    return encodeInfo(FQ, TWOS, BestWidth);
}

int Encoder::buildTable_EX()
{
    current_table.clear();
    vector<int> freqMap = DL->getFreqMap_2s();
    vector<pairInt> EXmap = DL->getExtremumMap_2s();
    vector<Info> valueInfoMap;
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.maxExtremum != b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        if(a.freq != b.freq)
            return a.freq > b.freq;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    TreeArray BestTree;
    vector<short> BestTable(16, 0);
    int BestWidth = INT32_MAX;
    #pragma omp parallel for num_threads(num_threads)
    for(int i=0;i<trees.size();i++){
        vector<short> tmpTable(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        int codeLength = 0;
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            tmpTable[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        int currentWidth = getMaxWidth_2s(tmpTable, i+trees.size());

        #pragma omp critical
        {   
            if(currentWidth < BestWidth){
                BestWidth = currentWidth;
                BestTree = trees[i];
                BestTable = tmpTable;
            }
        }
    }
    current_table = BestTable;
    if(BestWidth < global_best_width){
        global_best_width = BestWidth;
        global_best_table = BestTable;
        global_best_encodeInfo = encodeInfo(EX, TWOS, BestWidth);
    }
    return BestWidth;
}

encodeInfo Encoder::buildTable_EX(const set<int>& data_lines){
    current_table.clear();
    vector<int> freqMap = DL->getFreqMap_2s(data_lines);
    vector<pairInt> EXmap = DL->getExtremumMap_2s(data_lines);
    //sort the freqMap based on the frequency
    vector<Info> valueInfoMap;
    // <frequency, number>
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.maxExtremum != b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        if(a.freq != b.freq)
            return a.freq > b.freq;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    TreeArray BestTree;
    vector<short> BestTable(16, 0);
    int BestWidth = INT32_MAX;

    #pragma omp parallel for num_threads(num_threads)
    for(int i=0;i<trees.size();i++){
        //progressBar(i+1, trees.size());
        vector<short> tmpTable(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        // [nums of 0 bits code, nums of 1 bits code, ...] sum = 16
        int codeLength = 0;
        // let higher occurence symbol have shorter code length
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            tmpTable[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        int currentWidth = getMaxWidth_2s(tmpTable, data_lines);

        #pragma omp critical
        {   
            if(currentWidth < BestWidth){
                BestWidth = currentWidth;
                BestTree = trees[i];
                BestTable = tmpTable;
            }
        }
    }
    current_table = BestTable;
    return encodeInfo(EX, TWOS, BestWidth);
}

int Encoder::buildTable_FQ_SM(){
    current_table.clear();
    vector<int> freqMap = DL->getFreqMap_sm();
    vector<pairInt> EXmap = DL->getExtremumMap_sm();
    vector<Info> valueInfoMap;
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.freq != b.freq)
            return a.freq > b.freq;
        if(a.maxExtremum != b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    TreeArray BestTree;
    vector<short> BestTable(16,0);
    int BestWidth = INT32_MAX;

    #pragma omp parallel for num_threads(num_threads)
    for(int i=0;i<trees.size();i++){
        vector<short> tmpTable(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        int codeLength = 0;
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            tmpTable[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        int currentWidth = getMaxWidth_sm(tmpTable, i+2*trees.size());

        #pragma omp critical
        {   
            if(currentWidth < BestWidth){
                BestWidth = currentWidth;
                BestTree = trees[i];
                BestTable = tmpTable;
            }
        }
    }
    current_table = BestTable;
    if(BestWidth < global_best_width){
        global_best_width = BestWidth;
        global_best_table = BestTable;
        global_best_encodeInfo = encodeInfo(FQ, SM, BestWidth);
    }
    return BestWidth;
}

encodeInfo Encoder::buildTable_FQ_SM(const set<int>& data_lines){
    current_table.clear();
    vector<int> freqMap = DL->getFreqMap_sm(data_lines);
    vector<pairInt> EXmap = DL->getExtremumMap_sm(data_lines);
    //sort the freqMap based on the frequency
    vector<Info> valueInfoMap;
    // <frequency, number>
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.freq != b.freq)
            return a.freq > b.freq;
        if(a.maxExtremum != b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    TreeArray BestTree;
    vector<short> BestTable(16,0);
    int BestWidth = INT32_MAX;

    #pragma omp parallel for num_threads(num_threads)
    for(int i=0;i<trees.size();i++){
        //progressBar(i+1, trees.size());
        vector<short> tmpTable(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        // [nums of 0 bits code, nums of 1 bits code, ...] sum = 16
        int codeLength = 0;
        // let higher frequency symbol have shorter code length
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            tmpTable[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        int currentWidth = getMaxWidth_sm(tmpTable, data_lines);
        #pragma omp critical
        {   
            if(currentWidth < BestWidth){
                BestWidth = currentWidth;
                BestTree = trees[i];
                BestTable = tmpTable;
            }
        }
    }
    current_table = BestTable;
    return encodeInfo(FQ, SM, BestWidth);
}

int Encoder::buildTable_EX_SM(){
    current_table.clear();
    vector<int> freqMap = DL->getFreqMap_sm();
    vector<pairInt> EXmap = DL->getExtremumMap_sm();
    vector<Info> valueInfoMap;
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.maxExtremum != b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;    
        if(a.freq != b.freq)
            return a.freq > b.freq;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    TreeArray BestTree;
    vector<short> BestTable(16,0);
    int BestWidth = INT32_MAX;

    #pragma omp parallel for num_threads(num_threads)
    for(int i=0;i<trees.size();i++){
        vector<short> tmpTable(16,0);
        vector<short> codeTree = trees[i].getCodeArray();
        int codeLength = 0;
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            tmpTable[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        int  currentWidth = getMaxWidth_sm(tmpTable, i+3*trees.size());

        #pragma omp critical
        {   
            if(currentWidth < BestWidth){
                BestWidth = currentWidth;
                BestTree = trees[i];
                BestTable = tmpTable;
            }
        }
    }
    current_table = BestTable;
    if(BestWidth < global_best_width){
        global_best_width = BestWidth;
        global_best_table = BestTable;
        global_best_encodeInfo = encodeInfo(EX, SM, BestWidth);
    }
    return BestWidth;
}

encodeInfo Encoder::buildTable_EX_SM(const set<int>& data_lines){
    current_table.clear();
    vector<int> freqMap = DL->getFreqMap_sm(data_lines);
    vector<pairInt> EXmap = DL->getExtremumMap_sm(data_lines);
    vector<Info> valueInfoMap;
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.maxExtremum != b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;    
        if(a.freq != b.freq)
            return a.freq > b.freq;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    TreeArray BestTree;
    vector<short> BestTable(16,0);
    int BestWidth = INT32_MAX;

    #pragma omp parallel for num_threads(num_threads)
    for(int i=0;i<trees.size();i++){
        vector<short> tmpTable(16,0);
        vector<short> codeTree = trees[i].getCodeArray();
        int codeLength = 0;
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            tmpTable[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        int  currentWidth = getMaxWidth_sm(tmpTable,data_lines);

        #pragma omp critical
        {   
            if(currentWidth < BestWidth){
                BestWidth = currentWidth;
                BestTree = trees[i];
                BestTable = tmpTable;
            }
        }
    }
    current_table = BestTable;
    return encodeInfo(EX, SM, BestWidth);
}

void Encoder::buildTwoTable_FQ(int& GlobalBestWidth, vector<short>& table1, vector<short>& table2, vector<bool>& group){
    vector<int> freqMap = DL->getFreqMap_2s();
    vector<pairInt> EXmap = DL->getExtremumMap_2s();
    //sort the freqMap based on the frequency
    vector<Info> valueInfoMap;
    // <frequency, number>
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.freq != b.freq)
            return a.freq > b.freq;
        if(a.maxExtremum!=b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    for(int i=0;i<trees.size();i++){
        vector<short> t1(16, 0);
        vector<short> t2(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        vector<pair<int,unsigned short>> currentWidths(DL->getNumLines(), make_pair(0,0));
        vector<bool> g(DL->getNumLines(), false);
        encodeInfo e2;
        // make table
        int codeLength = 0;
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            t1[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        // make currentWidths
        #pragma omp parallel for num_threads(num_threads) if (DL->getNumLines() > 10000)
        for(int i=0;i<DL->getNumLines();i++){
            int tmp = 0;
            for(int j=0;j<16;j++){
                tmp += DL->getDataAry_freq_2s()[i][j]*t1[j];
            }
            currentWidths[i] = make_pair(i, tmp);
        }
        sort(currentWidths.begin(), currentWidths.end(), [](pair<int,unsigned short> a, pair<int,unsigned short> b){
            return a.second > b.second;
        });
        int max_width = currentWidths[0].second;
        int current = 0;
        set<int> linesForTable2;    
        
        while(current<TWOTABLELIMIT && current<DL->getNumLines()){
            linesForTable2.insert(currentWidths[current].first);
            vector<short> tmpTable;
            encodeInfo tmpInfo;
            tmpInfo = buildLocalBestTable(linesForTable2, tmpTable);
            if(tmpInfo.w <= max_width){
                max_width = max(currentWidths[current+1].second, tmpInfo.w);
                e2 = tmpInfo;
                t2 = tmpTable;
                g[currentWidths[current].first] = true;
                currentWidths[current].second = getWidth(t2, currentWidths[current].first, e2.e==TWOS);
                current++;
            }else{
                break;
            }
        }
        if(max_width < GlobalBestWidth){
            GlobalBestWidth = max_width;
            group = g;
            table1 = t1;
            table2 = t2;
            info1 = encodeInfo(FQ, TWOS, GlobalBestWidth);
            info2 = e2;
            num_use_tree = current-1;
        }
    }
}

void Encoder::buildTwoTable_EX(int& GlobalBestWidth, vector<short>& table1, vector<short>& table2, vector<bool>& group){
    vector<int> freqMap = DL->getFreqMap_2s();
    vector<pairInt> EXmap = DL->getExtremumMap_2s();
    //sort the freqMap based on the frequency
    vector<Info> valueInfoMap;
    // <frequency, number>
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.maxExtremum != b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        if(a.freq != b.freq)
            return a.freq > b.freq;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    for(int i=0;i<trees.size();i++){
        vector<short> t1(16, 0);
        vector<short> t2(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        vector<pair<int,unsigned short>> currentWidths(DL->getNumLines(), make_pair(0,0));
        vector<bool> g(DL->getNumLines(), false);
        encodeInfo e2;
        // make table
        int codeLength = 0;
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            t1[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        // make currentWidths
        #pragma omp parallel for num_threads(num_threads) if (DL->getNumLines() > 10000)
        for(int i=0;i<DL->getNumLines();i++){
            int tmp = 0;
            for(int j=0;j<16;j++){
                tmp += DL->getDataAry_freq_2s()[i][j]*t1[j];
            }
            currentWidths[i] = make_pair(i, tmp);
        }
        sort(currentWidths.begin(), currentWidths.end(), [](pair<int,unsigned short> a, pair<int,unsigned short> b){
            return a.second > b.second;
        });
        int max_width = currentWidths[0].second;
        int current = 0;
        set<int> linesForTable2;    
        while(current<TWOTABLELIMIT && current<DL->getNumLines()){
            linesForTable2.insert(currentWidths[current].first);
            vector<short> tmpTable;
            encodeInfo tmpInfo;
            tmpInfo = buildLocalBestTable(linesForTable2, tmpTable);

            if(tmpInfo.w <= max_width){
                max_width = tmpInfo.w > currentWidths[current+1].second?tmpInfo.w:currentWidths[current+1].second;
                e2 = tmpInfo;
                t2 = tmpTable;
                g[currentWidths[current].first] = true;
                currentWidths[current].second = getWidth(t2, currentWidths[current].first, e2.e==TWOS);
                current++;
            }else{
                break;
            }
        }
        if(max_width < GlobalBestWidth){
            GlobalBestWidth = max_width;
            group = g;
            table1 = t1;
            table2 = t2;
            info1 = encodeInfo(EX, TWOS, GlobalBestWidth);
            info2 = e2;
            num_use_tree = current-1;
        }
    }
}

void Encoder::buildTwoTable_FQ_SM(int& GlobalBestWidth, vector<short>& table1, vector<short>& table2, vector<bool>& group){
    vector<int> freqMap = DL->getFreqMap_sm();
    vector<pairInt> EXmap = DL->getExtremumMap_sm();
    //sort the freqMap based on the frequency
    vector<Info> valueInfoMap;
    // <frequency, number>
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.freq != b.freq)
            return a.freq > b.freq;
        if(a.maxExtremum!=b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    for(int i=0;i<trees.size();i++){
        vector<short> t1(16, 0);
        vector<short> t2(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        vector<pair<int,unsigned short>> currentWidths(DL->getNumLines(), make_pair(0,0));
        vector<bool> g(DL->getNumLines(), false);
        encodeInfo e2;
        // make table
        int codeLength = 0;
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            t1[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        // make currentWidths
        #pragma omp parallel for num_threads(num_threads) if (DL->getNumLines() > 10000)
        for(int i=0;i<DL->getNumLines();i++){
            int tmp = 0;
            for(int j=0;j<16;j++){
                tmp += DL->getDataAry_freq_sm()[i][j]*t1[j];
            }
            currentWidths[i] = make_pair(i, tmp);
        }
        sort(currentWidths.begin(), currentWidths.end(), [](pair<int,unsigned short> a, pair<int,unsigned short> b){
            return a.second > b.second;
        });
        int max_width = currentWidths[0].second;
        int current = 0;
        set<int> linesForTable2;    
        while(current<TWOTABLELIMIT && current<DL->getNumLines()){
            linesForTable2.insert(currentWidths[current].first);
            vector<short> tmpTable;
            encodeInfo tmpInfo;
            tmpInfo = buildLocalBestTable(linesForTable2, tmpTable);

            if(tmpInfo.w <= max_width){
                max_width = tmpInfo.w > currentWidths[current+1].second?tmpInfo.w:currentWidths[current+1].second;
                e2 = tmpInfo;
                t2 = tmpTable;
                g[currentWidths[current].first] = true;
                currentWidths[current].second = getWidth(t2, currentWidths[current].first, e2.e==TWOS);
                current++;
            }else{
                break;
            }
        }
        if(max_width < GlobalBestWidth){
            GlobalBestWidth = max_width;
            group = g;
            table1 = t1;
            table2 = t2;
            info1 = encodeInfo(FQ, SM, GlobalBestWidth);
            info2 = e2;
            num_use_tree = current-1;
        }
    }
}

void Encoder::buildTwoTable_EX_SM(int& GlobalBestWidth, vector<short>& table1, vector<short>& table2, vector<bool>& group){
    vector<int> freqMap = DL->getFreqMap_sm();
    vector<pairInt> EXmap = DL->getExtremumMap_sm();
    //sort the freqMap based on the frequency
    vector<Info> valueInfoMap;
    // <frequency, number>
    for(int i=0;i<16;i++){
        Info tmp(i, freqMap[i], EXmap[i].first, EXmap[i].second);
        valueInfoMap.push_back(tmp);
    }
    sort(valueInfoMap.begin(), valueInfoMap.end(), [](Info a, Info b){
        if(a.maxExtremum != b.maxExtremum)
            return a.maxExtremum > b.maxExtremum;
        if(a.freq != b.freq)
            return a.freq > b.freq;
        return a.minExtremum < b.minExtremum;
    });
    //build the table
    for(int i=0;i<trees.size();i++){
        vector<short> t1(16, 0);
        vector<short> t2(16, 0);
        vector<short> codeTree = trees[i].getCodeArray();
        vector<pair<int,unsigned short>> currentWidths(DL->getNumLines(), make_pair(0,0));
        vector<bool> g(DL->getNumLines(), false);
        encodeInfo e2;
        // make table
        int codeLength = 0;
        for(int j=0;j<16;j++){
            while(!codeTree[codeLength])
                codeLength++;
            t1[valueInfoMap[j].value] = codeLength;
            codeTree[codeLength]--;
        }
        // make currentWidths
        #pragma omp parallel for num_threads(num_threads) if (DL->getNumLines() > 10000)
        for(int i=0;i<DL->getNumLines();i++){
            int tmp = 0;
            for(int j=0;j<16;j++){
                tmp += DL->getDataAry_freq_sm()[i][j]*t1[j];
            }
            currentWidths[i] = make_pair(i, tmp);
        }
        sort(currentWidths.begin(), currentWidths.end(), [](pair<int,unsigned short> a, pair<int,unsigned short> b){
            return a.second > b.second;
        });
        int max_width = currentWidths[0].second;
        int current = 0;
        set<int> linesForTable2;    
        while(current<TWOTABLELIMIT && current<DL->getNumLines()){
            linesForTable2.insert(currentWidths[current].first);
            vector<short> tmpTable;
            encodeInfo tmpInfo;
            tmpInfo = buildLocalBestTable(linesForTable2, tmpTable);

            if(tmpInfo.w <= max_width){
                max_width = tmpInfo.w > currentWidths[current+1].second?tmpInfo.w:currentWidths[current+1].second;
                e2 = tmpInfo;
                t2 = tmpTable;
                g[currentWidths[current].first] = true;
                currentWidths[current].second = getWidth(t2, currentWidths[current].first, e2.e==TWOS);
                current++;
            }else{
                break;
            }
        }
        if(max_width < GlobalBestWidth){
            GlobalBestWidth = max_width;
            group = g;
            table1 = t1;
            table2 = t2;
            info1 = encodeInfo(EX, SM, GlobalBestWidth);
            info2 = e2;
            num_use_tree = current-1;
        }
    }
}

int Encoder::buildGlobalBestTable(){
    int current_width = buildTable_FQ();
    current_width = buildTable_EX();
    current_width = buildTable_FQ_SM();
    current_width = buildTable_EX_SM();
    return global_best_width;
}

encodeInfo Encoder::buildLocalBestTable(const set<int>& data_lines, vector<short>& table){
    int local_best_width = INT32_MAX;
    encodeInfo local_best_encodeInfo;
    encodeInfo current;
    current = buildTable_FQ(data_lines);
    if(current.w < local_best_width){
        table = current_table;
        local_best_width = current.w;
        local_best_encodeInfo = current;
    }
    current = buildTable_FQ_SM(data_lines);
    if(current.w < local_best_width){
        table = current_table;
        local_best_width = current.w;
        local_best_encodeInfo = current;
    }
    // Mostly, the set is small, so using EX method will lose standards and be inefficient.
    return local_best_encodeInfo;
}

int Encoder::buildTables(vector<short>& table1, vector<short>& table2, vector<bool>& group){
    int max_width = INT32_MAX;
    buildTwoTable_FQ(max_width, table1, table2, group);
    buildTwoTable_EX(max_width, table1, table2, group);
    buildTwoTable_FQ_SM(max_width, table1, table2, group);
    buildTwoTable_EX_SM(max_width, table1, table2, group);
    return max_width+1;
}

vector<short> Encoder::getTable()
{
    return global_best_table;
}

bool cmp(const pair<int, int> &a, const pair<int, int> &b)
{
    if (a.second == b.second)
        return a.first < b.first;
    return a.second < b.second;
}

vector<pair<int, int>> sort(vector<short> M)
{
    vector<pair<int, int>> A;
    for (int i = 0; i < M.size(); i++)
        A.push_back(make_pair(i, M[i]));
    sort(A.begin(), A.end(), cmp);
    return A;
}

bool checkTreeValid(vector<short> codeLengths)
{
    std::sort(codeLengths.begin(), codeLengths.end(), std::greater<int>());
    int currentLevel = codeLengths.front();
    int numNodesAtLevel = 0;
    for (int cl : codeLengths)
    {
        if (cl == 0)
            break;
        while (cl < currentLevel)
        {
            if (numNodesAtLevel % 2 != 0)
                throw std::invalid_argument("Under-full Huffman code tree");
            numNodesAtLevel /= 2;
            currentLevel--;
        }
        numNodesAtLevel++;
    }
    while (currentLevel > 0)
    {
        if (numNodesAtLevel % 2 != 0)
            throw std::invalid_argument("Under-full Huffman code tree");
        numNodesAtLevel /= 2;
        currentLevel--;
    }
    if (numNodesAtLevel < 1)
        throw std::invalid_argument("Under-full Huffman code tree");
    if (numNodesAtLevel > 1)
        throw std::invalid_argument("Over-full Huffman code tree");
    return true;
}

map<int, string> Encoder::genCanonCode(vector<short> table)
{
    // Check basic validity
    if (table.size() < 2)
        throw std::invalid_argument("At least 2 symbols needed");
    if (table.size() > UINT32_MAX)
        throw std::length_error("Too many symbols");
    checkTreeValid(table);
    // sort the codeLength map by value
    vector<pair<int, int>> sortedCodeLength = sort(table);
    map<int, string> canonCode;
    int prevCode = -1;
    int currCode = -1;
    string currCodeStr = "";
    int key = 0;
    int len = 0;
    int prevLen = 0;
    // start generating canonCode
    for (auto i = sortedCodeLength.begin(); i != sortedCodeLength.end(); i++)
    {
        key = i->first;
        len = i->second;
        if (len == 0)
        {
            canonCode[key] = "";
            continue;
        }
        if (prevCode == -1)
        {
            currCode = 0;
        }
        else
        {
            currCode = (prevCode + 1) << (len - prevLen);
        }
        canonCode[key] = bitset<40>(currCode).to_string().substr(40 - len, len);
        prevCode = currCode;
        prevLen = len;
    }
    return canonCode;
}

void Encoder::encode_oneT(string outputFileName)
{
    BitOutputStream *out = new BitOutputStream(outputFileName);
    int intPerLine = DL->getelementPerLine();
    int numLines = DL->getNumLines();
    out->writeInt(intPerLine);
    out->writeInt(numLines);
    bool is2s = global_best_encodeInfo.e == TWOS;
    // write the radix
    if(is2s)
        out->writeInt(0);
    else
        out->writeInt(1);
    // write the code length
    for (int i = 0; i < 16; i++)
        out->writeInt(global_best_table[i]);
    // write the code
    map<int, string> canonCode = genCanonCode(global_best_table);
    for (int i = 0; i < numLines; i++)
    {
        //progressBar("Encode", i, numLines - 1);
        for (int j = 0; j < intPerLine; j++){
            int num = DL->getDataAry()[i][j];
            if(is2s){
                int8 upper4bits_2s = (num >> 4) & 0x0F;
                int8 lower4bits_2s = num & 0x0F;
                out->writeCode(canonCode[upper4bits_2s]);
                out->writeCode(canonCode[lower4bits_2s]);
            }
            else{
                int8 s_M = ((num + (num >> 7)) ^ (num >> 7)) | (num & 0x80);
                int8 upper4bits_sm = (s_M >> 4) & 0x0F;
                int8 lower4bits_sm = s_M & 0x0F;
                out->writeCode(canonCode[upper4bits_sm]);
                out->writeCode(canonCode[lower4bits_sm]);
            }
        }
    }
    delete out;
}

void Encoder::encode_twoT(string outputFileName,vector<short> t1, vector<short>t2, vector<bool> group)
{
    BitOutputStream *out = new BitOutputStream(outputFileName);
    int intPerLine = DL->getelementPerLine();
    int numLines = DL->getNumLines();
    out->writeInt(intPerLine);
    out->writeInt(numLines);
    bool is2s_t1 = info1.e == TWOS;
    bool is2s_t2 = info2.e == TWOS;
    // write the radix
    if(is2s_t1)
        out->writeInt(0);
    else
        out->writeInt(1);
    // write the code length
    for (int i = 0; i < 16; i++)
        out->writeInt(t1[i]);
    if(is2s_t2)
        out->writeInt(0);
    else
        out->writeInt(1);
    for (int i = 0; i < 16; i++)
        out->writeInt(t2[i]);
    // write the code
    map<int, string> canonCode1 = genCanonCode(t1);
    map<int, string> canonCode2 = genCanonCode(t2);
    bool useG1 = true;
    int maxLength = 0;
    for (int i = 0; i < numLines; i++)
    {
        // t1 is 0, t2 is 1
        if(group[i]){
            out->writeBit(1);
            useG1 = false;
        }else{
            out->writeBit(0);
            useG1 = true;
        }
        int temp = 1;
        for (int j = 0; j < intPerLine; j++){
            // there are 4 cases
            // 1. use G1, 2s
            // 2. use G1, sm
            // 3. use G2, 2s
            // 4. use G2, sm
            int num = DL->getDataAry()[i][j];
            if(useG1){
                if(is2s_t1){
                    int8 upper4bits_2s = (num >> 4) & 0x0F;
                    int8 lower4bits_2s = num & 0x0F;
                    out->writeCode(canonCode1[upper4bits_2s]);
                    out->writeCode(canonCode1[lower4bits_2s]);
                    
                    temp += t1[upper4bits_2s];
                    temp += t1[lower4bits_2s];
                }else{
                    int8 s_M = ((num + (num >> 7)) ^ (num >> 7)) | (num & 0x80);
                    int8 upper4bits_sm = (s_M >> 4) & 0x0F;
                    int8 lower4bits_sm = s_M & 0x0F;
                    out->writeCode(canonCode1[upper4bits_sm]);
                    out->writeCode(canonCode1[lower4bits_sm]);
                    temp += t1[upper4bits_sm];
                    temp += t1[lower4bits_sm];
                }
            }else{
                if(is2s_t2){
                    int8 upper4bits_2s = (num >> 4) & 0x0F;
                    int8 lower4bits_2s = num & 0x0F;
                    out->writeCode(canonCode2[upper4bits_2s]);
                    out->writeCode(canonCode2[lower4bits_2s]);
                    temp += t2[upper4bits_2s];
                    temp += t2[lower4bits_2s];
                }else{
                    int8 s_M = ((num + (num >> 7)) ^ (num >> 7)) | (num & 0x80);
                    int8 upper4bits_sm = (s_M >> 4) & 0x0F;
                    int8 lower4bits_sm = s_M & 0x0F;
                    out->writeCode(canonCode2[upper4bits_sm]);
                    out->writeCode(canonCode2[lower4bits_sm]);
                    temp += t2[upper4bits_sm];
                    temp += t2[lower4bits_sm];
                }
            }
        }
        if(temp > maxLength)
            maxLength = temp;
    }

    delete out;
    max_encoding_length = maxLength;
}

Encoder::~Encoder()
{

}