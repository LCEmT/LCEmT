#include <iostream>
#include <vector>
#include <sys/mman.h>
#include "TreeArray.h"
#include "DataLoader.h"
#include "Encoder.h"
#include "Decoder.h"
#include "Utils.h"
#include <chrono>
#include <ctime> 
#include "Verification.h"

using namespace std;

int main(int argc, char *argv[])
{

    string dataFileName = argv[1];

    cout <<"Input Data: "<< dataFileName <<endl;
    string encodedFileName = dataFileName.substr(0, dataFileName.find_last_of('.')) + "_encoded.bin";
    string decodedFileName = dataFileName.substr(0, dataFileName.find_last_of('.')) + "_decoded.bin";
    
    auto start = std::chrono::system_clock::now();

    DataLoader *DL = new DataLoader(dataFileName);
    Encoder *encoder = new Encoder(TreeArray::getlegalTreesDP(16), DL);
    
    vector<short> table1(16, 0);
    vector<short> table2(16, 0);
    vector<bool> group(DL->getNumLines(), false);
    // Build Tables
    int compressedWidth = encoder->buildTables(table1,table2,group);
    int originalWidth = DL->getWidth();
    cout<<"Compression Rate: ";
    cout<<(double)100*(originalWidth-compressedWidth)/originalWidth<<"%"<<endl;
    // Encode Data
    encoder->encode_twoT(encodedFileName, table1, table2, group);
    // Decode and verification
    Decoder decoder(encodedFileName, decodedFileName, true);
    decoder.decode();
    bool success = verify_bin_quiet(DL, decodedFileName);
    if(success) {
        cout << "=== Verification passed ===" << endl;
        cout << "Compressed File Generated!" << endl;
        // remove decoded data
        remove(decodedFileName.c_str());
    } else {
        cout<< "Verification Failed" <<endl;
        cout<< "Please check the decoded data"<<endl;
    }
    

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s"
              << std::endl;
    return 0;
}

