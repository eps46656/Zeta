#include <zeta/core/define.h>

#include <fstream>
#include <iostream>

#define BLK_SIZE (512)

#define FBV_NAME "./fbv_2.fbv"

void main1() {
    std::fstream fs;
    fs.open(FBV_NAME, std::fstream::in | std::fstream::out |
                          std::fstream::binary | std::fstream::trunc);

    std::cout << fs.is_open() << "\n";

    if (!fs.is_open()) {
        std::cout << "fs is not open\n";
        return;
    }

    fs.seekg(0, std::fstream::end);

    size_t size = fs.tellg();
    ZETA_Core_PrintVar(size);

    fs.seekp(0, std::fstream::beg);

    for (size_t i = 0; i < 512 * 1024; ++i) { fs.put(0); }

    fs.close();

    std::cout << "write to " FBV_NAME "\n";
}

void main2() {
    FileBlockVector fbv;
    fbv.Open(FBV_NAME, BLK_SIZE);

    if (!fbv.IsOpen()) {
        std::cout << "fbv is not open\n";
        return;
    }

    ZETA_Core_PrintVar(fbv.blk_size);
    ZETA_Core_PrintVar(fbv.blk_num);
    ZETA_Core_PrintVar(fbv.blk_size * fbv.blk_num);
}

void main3() {
    FileBlockVector fbv;
    fbv.Open(FBV_NAME, BLK_SIZE);

    if (!fbv.IsOpen()) {
        std::cout << "fbv is not open\n";
        return;
    }

    ZETA_Core_PrintVar(fbv.blk_num);

    byte_t buffer[BLK_SIZE];

    for (int i{ 0 }; i < BLK_SIZE; ++i) { buffer[i] = i % 256; }

    Zeta_BlockVector bv;
    Zeta_BlockVector_Init(&bv);
    FileBlockVector_DeployBlockVector(&fbv, &bv);

    FileBlockVector_WriteBlock(&fbv, 1, buffer);

    fbv.Close();
}

int main() {
    main1();
    return 0;
}
