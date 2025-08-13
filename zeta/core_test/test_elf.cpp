#include <zeta/core/utils.h>

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

typedef unsigned long long ull;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define READ(dst, length)                                 \
    {                                                     \
        ZETA_Core_DebugAssert(data + length <= data_end); \
        dst = ReadFunc(data, length);                     \
        data += length;                                   \
    }                                                     \
    ZETA_StaticAssert(TRUE)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct ELF_Header {
    char ei_mag[4];

    byte_t ei_class;
    byte_t ei_data;

    byte_t ei_version;
    byte_t ei_osabi;
    byte_t ei_abiversion;

    u16_t e_type;
    u16_t e_machine;

    u32_t e_version;
    u64_t e_entry;
    u64_t e_phoff;
    u64_t e_shoff;

    u32_t e_flags;

    u16_t e_ehsize;

    u16_t e_phentsize;
    u16_t e_phnum;

    u16_t e_shentsize;
    u16_t e_shnum;

    u16_t e_shstrndx;
};

struct ELF_ProgramHeader {
    u32_t p_type;

    u32_t p_flags;

    u64_t p_offset;
    u64_t p_vaddr;
    u64_t p_paddr;
    u64_t p_filesz;
    u64_t p_memsz;

    u64_t p_align;
};

struct ELF_SectionHeader {
    u32_t sh_name;

    u32_t sh_type;

    u64_t sh_flags;

    u64_t sh_addr;
    u64_t sh_offset;
    u64_t sh_size;

    u32_t sh_link;
    u32_t sh_info;

    u64_t sh_addralign;
    u64_t sh_entsize;
};

struct ELF_Symbol {
    u64_t st_name;
    unsigned char st_info;
    unsigned char st_other;
    u64_t st_shndx;
    u64_t st_value;
    u64_t st_size;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct ELF_Program {
    ELF_ProgramHeader header;

    byte_t const* data;
    byte_t const* data_end;
};

struct ELF_Section {
    ELF_SectionHeader header;

    std::string name;

    byte_t const* data;
    byte_t const* data_end;
};

struct ELF {
    ELF_Header header;

    std::vector<ELF_Program> progs;
    std::vector<ELF_Section> sects;

    ELF_Section* shstr;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

byte_t const* ReadStr(std::string& dst, byte_t const* str_beg,
                      byte_t const* str_end) {
    for (;; ++str_beg) {
        ZETA_Core_DebugAssert(str_beg < str_end);
        char c = str_beg[0];
        if (c == '\0') { return str_beg + 1; }
        dst.push_back(c);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define TMP(dst, val, str) \
    case val: dst = str; break;

void ELF_PrintHeader(ELF_Header* header) {
    printf("ei_mag: %d %c(%d) %c(%d) %c(%d)\n", header->ei_mag[0],
           header->ei_mag[1], header->ei_mag[1], header->ei_mag[2],
           header->ei_mag[2], header->ei_mag[3], header->ei_mag[3]);

    printf("ei_class: %d bits\n", header->ei_class == 1 ? 32 : 64);

    printf("ei_data: %s\n",
           header->ei_data == 1 ? "little endian" : "big endian");

    printf("ei_version: %d\n", header->ei_version);

    char const* ei_osabi_str;

    switch (header->ei_version) {
        TMP(ei_osabi_str, 0x00, "System V");
        TMP(ei_osabi_str, 0x01, "HP-UX");
        TMP(ei_osabi_str, 0x02, "NetBSD");
        TMP(ei_osabi_str, 0x03, "Linux");
        TMP(ei_osabi_str, 0x04, "GNU Hurd");
        TMP(ei_osabi_str, 0x06, "Solaris");
        TMP(ei_osabi_str, 0x07, "AIX");
        TMP(ei_osabi_str, 0x08, "IRIX");
        TMP(ei_osabi_str, 0x09, "FreeBSD");
        TMP(ei_osabi_str, 0x0a, "Tru64");
        TMP(ei_osabi_str, 0x0b, "Movell Modesto");
        TMP(ei_osabi_str, 0x0c, "OpenBSD");
        TMP(ei_osabi_str, 0x0d, "OpenVMS");
        TMP(ei_osabi_str, 0x0e, "NonStop Kernel");
        TMP(ei_osabi_str, 0x0f, "AROS");
        TMP(ei_osabi_str, 0x10, "FenixOS");
        TMP(ei_osabi_str, 0x11, "Nuxi CloudABI");
        TMP(ei_osabi_str, 0x12, "Stratus Technologies OpenVOS");

        default: ei_osabi_str = "Unknown";
    }

    printf("ei_osabi: %d(%s)\n", header->ei_version, ei_osabi_str);
    printf("ei_abiversion: %d\n", header->ei_version);
    printf("\n");

    char const* e_type_str;

    switch (header->ei_version) {
        TMP(e_type_str, 0x00, "0x00 Unkown");
        TMP(e_type_str, 0x01, "Relocatable file");
        TMP(e_type_str, 0x02, "Executable file");
        TMP(e_type_str, 0x03, "Shared file");
        TMP(e_type_str, 0x04, "Core file");

        default: e_type_str = "Unkonwn"; break;
    }

    printf("e_type: %d(%s)\n", (int)header->e_type, e_type_str);
    printf("\n");

    char const* e_machine_str;

    switch (header->e_machine) {
        TMP(e_machine_str, 0x00, "No specific");
        TMP(e_machine_str, 0x01, "AT&T WE 32100");
        TMP(e_machine_str, 0x02, "SPARC");
        TMP(e_machine_str, 0x03, "x86");
        TMP(e_machine_str, 0x04, "Motorola 68000");
        TMP(e_machine_str, 0x05, "Motorola 88000");
        TMP(e_machine_str, 0x06, "Intel MCU");
        TMP(e_machine_str, 0x07, "Intel 8086");
        TMP(e_machine_str, 0x08, "MIPS");
        TMP(e_machine_str, 0x09, "IBM System/370");
        TMP(e_machine_str, 0x0a, "MIPS RS3000 Little-endian");

        TMP(e_machine_str, 0x14, "PowerPC");
        TMP(e_machine_str, 0x28, "Arm");
        TMP(e_machine_str, 0x32, "IA-64");
        TMP(e_machine_str, 0x3e, "AMD x86-64");

        TMP(e_machine_str, 0xf3, "RISC-V");

        default: e_machine_str = "Unknown"; break;
    }

    printf("e_machine: %d(0x%llX, %s)\n", (int)header->e_machine,
           (ull)header->e_machine, e_machine_str);

    printf("e_version: %lld\n", (ull)header->e_version);

    printf("\n");

    printf("e_entry: %llu(0x%llX)\n", (ull)header->e_entry,
           (ull)header->e_entry);

    printf("\n");

    printf("e_flags: 0x%llX\n", (ull)header->e_shoff);

    printf("e_ehsize: 0x%llX\n", (ull)header->e_ehsize);

    printf("\n");

    printf("e_phoff: %llu(0x%llX)\n", (ull)header->e_phoff,
           (ull)header->e_phoff);

    printf("e_phentsize: %llu(0x%llX)\n", (ull)header->e_phentsize,
           (ull)header->e_phentsize);

    printf("e_phnum: %llu\n", (ull)header->e_phnum);

    printf("\n");

    printf("e_shoff: %llu(0x%llX)\n", (ull)header->e_shoff,
           (ull)header->e_shoff);

    printf("e_shentsize: %llu(0x%llX)\n", (ull)header->e_shentsize,
           (ull)header->e_shentsize);

    printf("e_shnum: %llu\n", (ull)header->e_shnum);

    printf("\n");

    printf("e_shstrndx: %llu\n", (ull)header->e_shstrndx);
}

void ELF_PrintProgramHeader(ELF_ProgramHeader* prog_header) {
    char const* p_type_str;

    switch (prog_header->p_type) {
        TMP(p_type_str, 0x00000000, "PT_NULL");
        TMP(p_type_str, 0x00000001, "PT_LOAD");
        TMP(p_type_str, 0x00000002, "PT_DYNAMIC");
        TMP(p_type_str, 0x00000003, "PT_INTERP");
        TMP(p_type_str, 0x00000004, "PT_NOTE");
        TMP(p_type_str, 0x00000005, "PT_SHLIB");
        TMP(p_type_str, 0x00000006, "PT_PHDR");
        TMP(p_type_str, 0x00000007, "PT_TLS");
        TMP(p_type_str, 0x60000000, "PT_LOOS");
        TMP(p_type_str, 0x6fffffff, "PT_HIOS");
        TMP(p_type_str, 0x70000000, "PT_LOPROC");
        default: p_type_str = "Unknown"; break;
    }

    printf("p_type: %X (%s)\n", prog_header->p_type, p_type_str);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ELF_ReadHeader(ELF_Header* dst, byte_t const* data,
                    byte_t const* data_end) {
    u128_t (*ReadFunc)(byte_t const* data, unsigned length) =
        Zeta_ReadLittleEndian;

    dst->ei_mag[0] = data[0];
    dst->ei_mag[1] = data[1];
    dst->ei_mag[2] = data[2];
    dst->ei_mag[3] = data[3];
    data += 4;
    ZETA_Core_DebugAssert(data <= data_end);

    READ(dst->ei_class, 1);
    ZETA_Core_DebugAssert(dst->ei_class == 1 || dst->ei_class == 2);

    int length = dst->ei_class == 1 ? 4 : 8;

    READ(dst->ei_data, 1);
    ZETA_Core_DebugAssert(dst->ei_data == 1 || dst->ei_data == 2);

    if (dst->ei_data == 2) { ReadFunc = Zeta_ReadBigEndian; }

    READ(dst->ei_version, 1);
    READ(dst->ei_osabi, 1);
    READ(dst->ei_abiversion, 1);

    data += 7;
    ZETA_Core_DebugAssert(data <= data_end);

    READ(dst->e_type, 2);
    READ(dst->e_machine, 2);
    READ(dst->e_version, 4);

    READ(dst->e_entry, length);
    READ(dst->e_phoff, length);
    READ(dst->e_shoff, length);

    READ(dst->e_flags, 4);

    READ(dst->e_ehsize, 2);

    READ(dst->e_phentsize, 2);
    READ(dst->e_phnum, 2);

    READ(dst->e_shentsize, 2);
    READ(dst->e_shnum, 2);

    READ(dst->e_shstrndx, 2);
}

void ELF_ReadProgramHeader(ELF_ProgramHeader* dst, ELF_Header* header,
                           byte_t const* data, byte_t const* data_end) {
    ZETA_Core_DebugAssert(data <= data_end);

    u128_t (*ReadFunc)(byte_t const* data, unsigned length) =
        header->ei_data == 1 ? Zeta_ReadLittleEndian : Zeta_ReadBigEndian;

    int length = header->ei_class == 1 ? 4 : 8;

    READ(dst->p_type, 4);

    if (length == 8) { READ(dst->p_flags, 4); }

    READ(dst->p_offset, length);
    READ(dst->p_vaddr, length);
    READ(dst->p_paddr, length);
    READ(dst->p_filesz, length);
    READ(dst->p_memsz, length);

    if (length == 4) { READ(dst->p_flags, 4); }

    READ(dst->p_align, length);
}

void ELF_ReadSectionHeader(ELF_SectionHeader* dst, ELF_Header* header,
                           byte_t const* data, byte_t const* data_end) {
    ZETA_Core_DebugAssert(data <= data_end);

    u128_t (*ReadFunc)(byte_t const* data, unsigned length) =
        header->ei_data == 1 ? Zeta_ReadLittleEndian : Zeta_ReadBigEndian;

    int length = header->ei_class == 1 ? 4 : 8;

    READ(dst->sh_name, 4);
    READ(dst->sh_type, 4);

    READ(dst->sh_flags, length);

    READ(dst->sh_addr, length);
    READ(dst->sh_offset, length);
    READ(dst->sh_size, length);

    READ(dst->sh_link, 4);
    READ(dst->sh_info, 4);

    READ(dst->sh_addralign, length);
    READ(dst->sh_entsize, length);
}

void ELF_ReadSymbol(ELF_Symbol* dst, ELF_Header* header, byte_t const* data,
                    byte_t const* data_end) {
    ZETA_Core_DebugAssert(data <= data_end);

    u128_t (*ReadFunc)(byte_t const* data, unsigned length) =
        header->ei_data == 1 ? Zeta_ReadLittleEndian : Zeta_ReadBigEndian;

    int length = header->ei_class == 1 ? 4 : 8;

    READ(dst->st_name, length);

    if (length == 4) {
        READ(dst->st_value, length);
        READ(dst->st_size, length);
    }

    READ(dst->st_info, 1);
    READ(dst->st_other, 1);
    READ(dst->st_shndx, length);

    if (length == 8) {
        READ(dst->st_value, length);
        READ(dst->st_size, length);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ELF_Read(ELF* elf, byte_t const* data, byte_t const* data_end) {
    ZETA_Core_DebugAssert(data <= data_end);

    ELF_ReadHeader(&elf->header, data, data_end);

    elf->progs.resize(elf->header.e_phnum);

    for (int i = 0; i < elf->header.e_phnum; ++i) {
        ELF_ReadProgramHeader(
            &elf->progs[i].header, &elf->header,
            data + elf->header.e_phoff + elf->header.e_phentsize * i, data_end);

        byte_t const* prog_data = data + elf->progs[i].header.p_offset;
        byte_t const* prog_data_end = prog_data + elf->progs[i].header.p_filesz;

        ZETA_Core_DebugAssert(prog_data_end <= data_end);

        elf->progs[i].data = prog_data;
        elf->progs[i].data_end = prog_data_end;
    }

    elf->sects.resize(elf->header.e_shnum);

    for (int i = 0; i < elf->header.e_shnum; ++i) {
        ELF_ReadSectionHeader(
            &elf->sects[i].header, &elf->header,
            data + elf->header.e_shoff + elf->header.e_shentsize * i, data_end);

        byte_t const* sect_data = data + elf->sects[i].header.sh_offset;
        byte_t const* sect_data_end = sect_data + elf->sects[i].header.sh_size;

        printf("%d: %p -> %p\n", i, sect_data, sect_data_end);

        ZETA_Core_DebugAssert(sect_data_end <= data_end);

        elf->sects[i].data = sect_data;
        elf->sects[i].data_end = sect_data_end;
    }

    size_t e_shstrndx = elf->header.e_shstrndx;

    ZETA_Core_DebugAssert(0 <= e_shstrndx);
    ZETA_Core_DebugAssert(e_shstrndx < elf->header.e_shnum);

    elf->shstr = &elf->sects[e_shstrndx];

    ZETA_Core_PrintVar(e_shstrndx);
    ZETA_Core_PrintVar(elf->shstr->data);
    ZETA_Core_PrintVar(elf->shstr->data_end);

    for (byte_t const* iter = elf->shstr->data; iter != elf->shstr->data_end;
         ++iter) {
        std::cout << *(char const*)iter;
    }

    printf("\n");

    for (int i = 0; i < elf->header.e_shnum; ++i) {
        ReadStr(elf->sects[i].name,
                elf->shstr->data + elf->sects[i].header.sh_name,
                elf->shstr->data_end);

        printf("%d: %s\n", i, elf->sects[i].name.c_str());
    }

    std::unordered_map<std::string, size_t> name_to_idx;

    for (size_t i = 0; i < elf->header.e_shnum; ++i) {
        bool_t b = name_to_idx.insert({ elf->sects[i].name, i }).second;
        ZETA_Core_DebugAssert(b);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int main1() {
    std::fstream iofs{ "./test_2.elf", std::ios::in | std::ios::binary };

    std::vector<byte_t> data_v;
    while (iofs) { data_v.push_back((byte_t)iofs.get()); }

    ZETA_Core_PrintVar("%zu\n", data_v.size());

    byte_t const* data = data_v.data();
    byte_t const* data_end = data_v.data() + data_v.size();

    ELF_Header header;

    ELF_ReadHeader(&header, data, data_end);

    ELF_PrintHeader(&header);

    // return 0;

    std::vector<ELF_ProgramHeader> prog_headers;
    prog_headers.resize(header.e_phnum);

    for (int i = 0; i < header.e_phnum; ++i) {
        ELF_ReadProgramHeader(&prog_headers[i], &header,
                              data + header.e_phoff + header.e_phentsize * i,
                              data_end);

        ELF_PrintProgramHeader(&prog_headers[i]);

        // ZETA_Core_PrintVar("%llx", (ull)prog_headers[i].p_type);
    }

    std::vector<ELF_SectionHeader> sect_headers;
    sect_headers.resize(header.e_shnum);

    for (int i = 0; i < header.e_shnum; ++i) {
        ELF_ReadSectionHeader(&sect_headers[i], &header,
                              data + header.e_shoff + header.e_shentsize * i,
                              data_end);

        ZETA_Core_PrintVar("%llx", (ull)sect_headers[i].sh_type);
    }

    printf("ok\n");

    return 0;
}

int main2() {
    std::fstream iofs{ "./test_2.elf", std::ios::in | std::ios::binary };

    std::vector<byte_t> data_v;
    while (iofs) { data_v.push_back((byte_t)iofs.get()); }

    ZETA_Core_PrintVar("%zu\n", data_v.size());

    byte_t const* data = data_v.data();
    byte_t const* data_end = data_v.data() + data_v.size();

    ELF elf;

    ELF_Read(&elf, data, data_end);

    printf("%lld\n", elf.header.ei_mag[0]);
    printf("%lld\n", elf.header.ei_mag[1]);
    printf("%lld\n", elf.header.ei_mag[2]);
    printf("%lld\n", elf.header.ei_mag[3]);

    printf("ok\n");

    return 0;
}

int main() { return main2(); }
