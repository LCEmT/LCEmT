#include "Decoder.h"
#include "CodeTree.h"
#include "Utils.h"
#include <stdexcept>
#include <vector>
#include <sstream>
#include <algorithm>



CodeTree *Decoder::CanonicalToCodeTree(vector<int> codeLengths)
{
    vector<std::unique_ptr<Node>> nodes;
    for (int i = *std::max_element(codeLengths.cbegin(), codeLengths.cend());; i--)
    { // Descend through code lengths
        if (nodes.size() % 2 != 0)
            throw std::logic_error("Assertion error: Violation of canonical code invariants");
        vector<std::unique_ptr<Node>> newNodes;

        // Add leaves for symbols with positive code length i
        if (i > 0)
        {
            int j = 0;
            for (int cl : codeLengths)
            {
                if (cl == i)
                    newNodes.push_back(std::unique_ptr<Node>(new Leaf(j)));
                j++;
            }
        }

        // Merge pairs of nodes from the previous deeper layer
        for (std::size_t j = 0; j < nodes.size(); j += 2)
        {
            newNodes.push_back(std::unique_ptr<Node>(new InternalNode(
                std::move(nodes.at(j)), std::move(nodes.at(j + 1)))));
        }
        nodes = std::move(newNodes);

        if (i == 0)
            break;
    }

    if (nodes.size() != 1)
        throw std::logic_error("Assertion error: Violation of canonical code invariants");

    Node *temp = nodes.front().release();
    InternalNode *root = dynamic_cast<InternalNode *>(temp);
    CodeTree *result = new CodeTree(std::move(*root), static_cast<int>(codeLengths.size()));
    delete root;
    return result;
}

Decoder::Decoder(string encodedFileName, string decodedFileName, bool twoTree) : in(encodedFileName), out(decodedFileName), twoTree(twoTree)
{
    max_line_width = 0;
    intPerLine = in.readInt();
    numLines = in.readInt();
    vector<int> codeLengths(16,0);
    int temp = in.readInt();
    if(temp == 0){
        is2s = true;
    }else if(temp == 1){
        is2s = false;
    }else{
        throw std::logic_error("Assertion error: Invalid value from input.get()");
    }
    
    for(int i=0;i<16;i++){
        codeLengths[i] = in.readInt();
    }
    codeTree = CanonicalToCodeTree(codeLengths);
    if(!twoTree){
        codeTree_2 = nullptr;
    }else{
        vector<int> codeLengths_2(16,0);
        temp = in.readInt();
        if(temp == 0){
            is2s_2 = true;
        }else if(temp == 1){
            is2s_2 = false;
        }else{
            throw std::logic_error("Assertion error: Invalid value from input.get()");
        }
        for(int i=0;i<16;i++){
            codeLengths_2[i] = in.readInt();
        }
        codeTree_2 = CanonicalToCodeTree(codeLengths_2);
    }
}

int Decoder::decodeOne(int &line_width, bool useSecondTree)
{
    if (codeTree == nullptr)
        throw std::logic_error("Code tree is null");

    const InternalNode *currentNode = &codeTree->root;
    if(useSecondTree){
        currentNode = &codeTree_2->root;
    }
    int num = 0;
    int result = 0;
    int8 n1, n2;

    while (true)
    {
        int temp = in.readBit();
        const Node *nextNode;
        if (temp == 0)
            nextNode = currentNode->leftChild.get();
        else if (temp == 1)
            nextNode = currentNode->rightChild.get();
        else
            throw std::logic_error("End of input before code completed");

        line_width++;

        if (dynamic_cast<const Leaf *>(nextNode) != nullptr)
        {
            if(num == 0){
                num = 1;
                n1 = dynamic_cast<const Leaf *>(nextNode)->symbol;
            }else{
                num = 0;
                n2 = dynamic_cast<const Leaf *>(nextNode)->symbol;
                if((is2s&&!useSecondTree) || (is2s_2&&useSecondTree)){
                    int rt = (n1 << 4) + n2;
                    rt = (rt << 24) >> 24;
                    result = rt;
                }else{
                    int rt = (n1 << 4) + n2;
                    rt = (rt>>7) ? 0x80-rt : rt;
                    if(n1==8&&n2==0)
                        rt = -128;
                    result = rt;
                }
                return result;
            } 
            if(useSecondTree)
                currentNode = &codeTree_2->root;
            else
                currentNode = &codeTree->root;           
        }
        else if (dynamic_cast<const InternalNode *>(nextNode))
            currentNode = dynamic_cast<const InternalNode *>(nextNode);
        else
            throw std::logic_error("Assertion error: Illegal node type");
    }
}

void Decoder::decode()
{    
    out.write((char *)&intPerLine, sizeof(int));
    out.write((char *)&numLines, sizeof(int));

    int8 tmp = 0;
    int line_width = 0;
    
    for (int i = 0; i < numLines; i++)
    {
        line_width = 0;
        if(twoTree){
            // read first bit
            int fb = in.readBit();
            for (int j = 0; j < intPerLine; j++)
            {
                tmp = decodeOne(line_width,fb);
                out.write((char *)&tmp, sizeof(int8));
            }
        }else{
            for (int j = 0; j < intPerLine; j++)
            {
                tmp = decodeOne(line_width,false);
                out.write((char *)&tmp, sizeof(int8));
            }
        }      
        // cout<<"line width: "<<line_width<<endl;
        max_line_width = max(max_line_width, line_width+1);
    }
    out.close();
}
