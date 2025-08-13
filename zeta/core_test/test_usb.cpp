#include <unistd.h>
#include <zeta/core/define.h>
#include <zeta/core/disk_info.h>
#include <zeta/core/disk_part_gpt.h>
#include <zeta/core/disk_part_mbr.h>
#include <zeta/core/utf16.h>
#include <zeta/core/utf8.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#define lengthof(arr) (sizeof(arr) / sizeof(arr[0]))

#define SEC_SIZE (512)

#define PRINT(var)                                                     \
    std::cout << __FILE__ << ":" << __LINE__ << "    " << #var << ": " \
              << (var) << "\n";

#define PRINT_LINE std::cout << "\n--------------------\n\n";

using ull_t = unsigned long long;

void ReadSec(byte_t* dst, std::fstream& fs, size_t sec_size, size_t sec_idx) {
    fs.seekg(sec_size * sec_idx, std::fstream::beg);
    for (size_t i = 0; i < sec_size; ++i) { dst[i] = fs.get(); }
}

void ReadFromFS(byte_t* dst, std::fstream& fs, size_t beg, size_t length) {
    fs.seekg(beg, std::fstream::beg);
    for (size_t i = 0; i < length; ++i) { dst[i] = fs.get(); }
}

void PrintMBRPartEntry(Zeta_Core_DiskPartMBR_PartEntry* entry) {
    std::cout << "state: " << (ull_t)entry->state << "\n";
    std::cout << "type: " << (ull_t)entry->type << "    " << std::hex
              << (ull_t)entry->type << "\n";
    std::cout << "beg: " << (ull_t)entry->beg << "\n";
    std::cout << "end: " << (ull_t)entry->end << "\n";
}

void PrintGPTHeader(Zeta_Core_DiskPartGPT_Header* gpt_header) {
    printf("revision num: %zu\n", gpt_header->revision_num);
    printf("header size: %zu\n", gpt_header->header_size);
    printf("crc32: %lld\n", (ull_t)gpt_header->header_crc32);
    printf("cur_lba: %lld\n", (ull_t)gpt_header->cur_lba);
    printf("bk_lba: %lld\n", (ull_t)gpt_header->bk_lba);

    printf("disk_guid: ");

    for (int i = 0; i < 16; ++i) { printf("%X ", gpt_header->disk_guid[i]); }

    printf("\n");

    printf("beg_of_part_entries: %llu\n",
           (ull_t)gpt_header->beg_of_part_entries);
    printf("num_of_part_entries: %llu\n",
           (ull_t)gpt_header->num_of_part_entries);
    printf("size_of_part_entry: %llu\n", (ull_t)gpt_header->size_of_part_entry);
}

void PrintGPTPartEntry(Zeta_Core_DiskPartGPT_PartEntry* gpt_part_entry) {
    for (int i = 0; i < 16; ++i) {
        if (gpt_part_entry->type_guid[i] != 0) { goto L1; }
    }

    return;

L1:

    PRINT_LINE;

    printf("type_guid: ");

    for (int i = 0; i < 16; ++i) {
        printf("%X ", gpt_part_entry->type_guid[i]);
    }

    printf("\n");

    printf("part_guid: ");

    for (int i = 0; i < 16; ++i) {
        printf("%X ", gpt_part_entry->part_guid[i]);
    }

    printf("\n");

    printf("beg: %llu\n", (ull_t)gpt_part_entry->beg);
    printf("end: %llu\n", (ull_t)gpt_part_entry->end);

    unichar_t uni_str[36];

    {
        UTF16_DecodeRet r = Zeta_Core_UTF16_Decode(
            uni_str, uni_str + lengthof(uni_str), gpt_part_entry->name,
            gpt_part_entry->name + lengthof(gpt_part_entry->name), TRUE);

        ZETA_Core_DebugAssert(r.success);
        ZETA_Core_DebugAssert(r.nxt_data == gpt_part_entry->name +
                                                lengthof(gpt_part_entry->name));
    }

    byte_t utf8_str[36 * 6];

    {
        Zeta_Core_UTF8_EncodeRet r =
            Zeta_Core_UTF8_Encode(utf8_str, utf8_str + lengthof(utf8_str),
                                  uni_str, uni_str + lengthof(uni_str));

        ZETA_Core_DebugAssert(r.success);
    }

    printf("name: ");

    for (size_t i = 0; i < lengthof(utf8_str); ++i) {
        if (utf8_str[i] == '\0') { break; }
        printf("%c", utf8_str[i]);
    }

    printf("\n");

    PRINT_LINE;
}

void main_export() {
    char const devpath[] = "/dev/sda";

    std::cout << "devpath: " << devpath << "\n";

    std::fstream fs;
    fs.open(devpath, std::ios_base::in | std::ios_base::binary);

    if (!fs) {
        std::cout << "open failed\n";
        std::cout << std::strerror(errno) << "\n";
        return;
    }

    std::cout << "open success\n";

    char const iso_path[] = "./2022_0228_1654.iso";

    std::fstream ofs;
    ofs.open(iso_path, std::ios_base::out | std::ios_base::binary);

    if (!ofs) {
        std::cout << "open failed\n";
        std::cout << std::strerror(errno) << "\n";
        return;
    }

    for (;;) {
        int c = fs.get();
        if (fs.eof()) { break; }
        ofs.put(c);
    }

    fs.close();
    ofs.close();
}

void main1() {
    //  char const devpath[] = "/dev/sda";

    // std::cout << "devpath: " << devpath << "\n";

    std::fstream fs;
    fs.open("./2022_0228_1654.iso", std::ios_base::in | std::ios_base::binary);

    if (!fs) {
        std::cout << "open failed\n";
        std::cout << std::strerror(errno) << "\n";
        return;
    }

    std::cout << "open success\n";

    // std::cout << ""

    fs.seekg(0, std::fstream::end);

    size_t dev_size = fs.tellg();

    ZETA_Core_DebugAssert(dev_size % SEC_SIZE == 0);

    ZETA_Core_PrintVar(dev_size);

    size_t num_of_secs = dev_size / SEC_SIZE;

    ZETA_Core_PrintVar(num_of_secs);

    Zeta_Core_DiskInfo disk_info = (Zeta_Core_DiskInfo){
        .sec_size = SEC_SIZE,
        .num_of_secs = num_of_secs,

        .num_of_cylinders = 1,
        .heads_per_cylinder = 1,
        .secs_per_track = num_of_secs,
    };

    byte_t buff[SEC_SIZE];
    byte_t const* data;

    ReadSec(buff, fs, SEC_SIZE, 0);

    Zeta_Core_DiskPartMBR_MBR mbr;

    data = Zeta_Core_DiskPartMBR_ReadMBR(&mbr, &disk_info, buff,
                                         buff + lengthof(buff));

    ZETA_Core_DebugAssert(data != NULL);

    if (data == NULL) {
        std::cout << "failed mbr read\n";
        return;
    }

    /*
    PrintMBRPartEntry(&mbr.part_entries[0]);
    PRINT_LINE;
    PrintMBRPartEntry(&mbr.part_entries[1]);
    PRINT_LINE;
    PrintMBRPartEntry(&mbr.part_entries[2]);
    PRINT_LINE;
    PrintMBRPartEntry(&mbr.part_entries[3]);
    PRINT_LINE;
    */

    ReadSec(buff, fs, SEC_SIZE, 1);

    Zeta_Core_DiskPartGPT_Header gpt_header;

    data = Zeta_Core_DiskPartGPT_ReadHeader(&gpt_header, buff,
                                            buff + lengthof(buff));

    ZETA_Core_DebugAssert(data != NULL);

    PrintGPTHeader(&gpt_header);

    ZETA_Core_DebugAssert(gpt_header.size_of_part_entry <= sizeof(buff));

    size_t part_entry_i = SEC_SIZE * gpt_header.beg_of_part_entries;

    for (size_t i = 0; i < gpt_header.num_of_part_entries; ++i) {
        ReadFromFS(buff, fs, part_entry_i, gpt_header.size_of_part_entry);

        Zeta_Core_DiskPartGPT_PartEntry gpt_part_entry;
        Zeta_Core_DiskPartGPT_ReadPartEntry(&gpt_part_entry, buff,
                                            buff + lengthof(buff));

        PrintGPTPartEntry(&gpt_part_entry);

        part_entry_i += gpt_header.size_of_part_entry;
    }
}

/*
void main2() {
    int fd = open("/dev/sda", O_ASYNC);

    std::cout << fd << "\n";

    close(fd);
}
*/

int main() {
    main1();
    std::cout << "ok\n";
    return 0;
}
