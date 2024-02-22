#include <iostream>
typedef char int8;
int main() {
    int num = -77; // Represented in two's complement as 111...1000
    
    int8 s_M = ((num + (num >> 7)) ^ (num >> 7)) | (num & 0x80);
    int8 upper4bits_2s = (num >> 4) & 0x0F;
    int8 lower4bits_2s = num & 0x0F;
    int8 upper4bits_sm = (s_M >> 4) & 0x0F;
    int8 lower4bits_sm = s_M & 0x0F;
    // restore num from upper4bits_2s and lower4bits_2s
    int rs = (upper4bits_2s << 4) + lower4bits_2s;
    // sign extenstion for 8 bit rs
    rs = (rs << 24) >> 24;
    // restore num from upper4bits_sm and lower4bits_sm
    int rs2 = (upper4bits_sm << 4) + lower4bits_sm;
    // transform rs2 from sign and magnitude to 2s
    /*
(in >> 31) ? 0x80000000-in : in
    */
    rs2 = (rs2>>7) ? 0x80-rs2 : rs2;
    //rs2 = (rs2 << 24) >> 24;

    std::cout << int(s_M) << std::endl;
    std::cout << int(rs2) << std::endl;
    std::cout << int(upper4bits_sm) << std::endl;
    std::cout << int(lower4bits_sm) << std::endl;
    std::cout << int(upper4bits_2s) << std::endl;
    std::cout << int(lower4bits_2s) << std::endl;

    return 0;
}
