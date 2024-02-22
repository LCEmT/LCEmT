#include "TreeArray.h"
#include <math.h>

vector<vector<TreeArray>> TreeArray::legalTreeMemoryTable = vector<vector<TreeArray>>(1, vector<TreeArray>(1));

TreeArray::TreeArray(vector<short> arr){
    array = arr;
    numLeaf = 0;
    for(int i=0;i<array.size();i++){
        numLeaf += array[i];
    }
    maxCodeLength = -1;
    minCodeLength = -1;
    maxCodeLength = getMaxCodeLength();
    minCodeLength = getMinCodeLength();
}

TreeArray::TreeArray(): array(ARRAY_SIZE, 0), numLeaf(1)
{
    array[0] = 1;
    maxCodeLength = 0;
    minCodeLength = 0;
}
TreeArray::TreeArray(int numLeaf): array(ARRAY_SIZE, 0), numLeaf(numLeaf)
{
    maxCodeLength = -1;
    minCodeLength = -1;
    if(numLeaf==1){
        array[0] = 1;
        maxCodeLength = 0;
        minCodeLength = 0;
    }    
}

vector<TreeArray> TreeArray::getlegalTreesDP(int numLeaf){
    if(numLeaf <= legalTreeMemoryTable.size()){
        return legalTreeMemoryTable[numLeaf-1];
    }else{
        for(int i=legalTreeMemoryTable.size();i<numLeaf;i++){
            legalTreeMemoryTable.push_back(getlegalTrees(i+1));
        }
        return legalTreeMemoryTable[numLeaf-1];
    }
}

vector<TreeArray> TreeArray::getlegalTrees(int numLeaf){
    vector<TreeArray> legalTrees;
    if(numLeaf == 1){
        legalTrees.push_back(TreeArray(numLeaf));
        return legalTrees;
    }
    
    for(int i = 1; i < numLeaf; i++){
        int left = i;
        int right = numLeaf - i;
        vector<TreeArray> leftLegalTrees = getlegalTreesDP(left);
        vector<TreeArray> rightLegalTrees = getlegalTreesDP(right);
        for(int j=0;j<leftLegalTrees.size();j++){
            for(int k=0;k<rightLegalTrees.size();k++){
                //max code length of left tree <= min code length of right tree
                if(leftLegalTrees[j].getMaxCodeLength() > rightLegalTrees[k].getMinCodeLength()){
                    continue;
                }
                legalTrees.push_back(mergeTree(leftLegalTrees[j], rightLegalTrees[k]));
            }
        }
    }
    return legalTrees;
}

int TreeArray::getMaxCodeLength(){
    if(maxCodeLength != -1){
        return maxCodeLength;
    }
    for(int i=array.size()-1;i>=0;i--){
        if(array[i] != 0){
            maxCodeLength = i;
            return maxCodeLength;
        }
    }
    return -1;
}
int TreeArray::getMinCodeLength(){
    if(minCodeLength != -1){
        return minCodeLength;
    }
    for(int i=0;i<array.size();i++){
        if(array[i] != 0){
            minCodeLength = i;
            return minCodeLength;
        }
    }
    return -1;
}

TreeArray TreeArray::mergeTree(TreeArray& leftTree, TreeArray& rightTree){
    vector<short> array(ARRAY_SIZE, 0);
    for(int i=0;i<array.size();i++){
        array[i] = leftTree.array[i] + rightTree.array[i];
    }
    //right shift array
    for(int i=array.size()-1;i>0;i--){
        array[i] = array[i-1];
    }
    array[0] = 0;
    return TreeArray(array);
}

bool TreeArray::checkKraft(){
    double sum = 0;
    for(int i=0;i<array.size();i++){
        sum += array[i] * pow(2, -i);
    }
    return (int)sum == 1;
}

vector<short> TreeArray::getCodeArray(){
    return array;
}

ostream& operator<<(ostream& os, const TreeArray& tree)
{
    for (int i = 0; i < tree.array.size(); i++)
    {
        os << tree.array[i] << " ";
    }
    return os;
}