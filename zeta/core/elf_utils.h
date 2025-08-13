#pragma once

#include <zeta/core/seq_cntr.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_ELF_Ret);
ZETA_Core_DeclareStruct(Zeta_Core_ELF_Header);
ZETA_Core_DeclareStruct(Zeta_Core_ELF_ProgramHeader);
ZETA_Core_DeclareStruct(Zeta_Core_ELF_SectionHeader);
ZETA_Core_DeclareStruct(Zeta_Core_ELF_Symbol);
ZETA_Core_DeclareStruct(Zeta_Core_ELF);

#define ZETA_Core_ELF_PT_NULL 0
#define ZETA_Core_ELF_PT_LOAD 1
#define ZETA_Core_ELF_PT_DYNAMIC 2
#define ZETA_Core_ELF_PT_INTERP 3
#define ZETA_Core_ELF_PT_NOTE 4
#define ZETA_Core_ELF_PT_SHLIB 5
#define ZETA_Core_ELF_PT_PHDR 6
#define ZETA_Core_ELF_PT_LOSUNW 0x6ffffffa
#define ZETA_Core_ELF_PT_SUNWBSS 0x6ffffffb
#define ZETA_Core_ELF_PT_SUNWSTACK 0x6ffffffa
#define ZETA_Core_ELF_PT_HISUNW 0x6fffffff
#define ZETA_Core_ELF_PT_LOPROC 0x70000000
#define ZETA_Core_ELF_PT_HIPROC 0x7fffffff

#define ZETA_Core_ELF_SHT_NULL 0
#define ZETA_Core_ELF_SHT_PROGBITS 1
#define ZETA_Core_ELF_SHT_SYMTAB 2
#define ZETA_Core_ELF_SHT_STRTAB 3
#define ZETA_Core_ELF_SHT_RELA 4
#define ZETA_Core_ELF_SHT_HASH 5
#define ZETA_Core_ELF_SHT_DYNAMIC 6
#define ZETA_Core_ELF_SHT_NOTE 7
#define ZETA_Core_ELF_SHT_NOBITS 8
#define ZETA_Core_ELF_SHT_REL 9
#define ZETA_Core_ELF_SHT_SHLIB 10
#define ZETA_Core_ELF_SHT_DYNSYM 11
#define ZETA_Core_ELF_SHT_LOPROC 0x70000000
#define ZETA_Core_ELF_SHT_HIPROC 0x7fffffff
#define ZETA_Core_ELF_SHT_LOUSER 0x80000000
#define ZETA_Core_ELF_SHT_HIUSER 0xffffffff

struct Zeta_Core_ELF_Ret {
    int rc;
    byte_t* nxt_dst;
    byte_t const* nxt_data;
};

struct Zeta_Core_ELF_Header {
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

struct Zeta_Core_ELF_ProgramHeader {
    u32_t p_type;

    u32_t p_flags;

    u64_t p_offset;
    u64_t p_vaddr;
    u64_t p_paddr;
    u64_t p_filesz;
    u64_t p_memsz;

    u64_t p_align;
};

struct Zeta_Core_ELF_SectionHeader {
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

struct Zeta_Core_ELF_Symbol {
    u64_t st_name;
    unsigned char st_info;
    unsigned char st_other;
    u64_t st_shndx;
    u64_t st_value;
    u64_t st_size;
};

struct Zeta_Core_ELF {
    Zeta_Core_ELF_Header header;

    Zeta_Core_SeqCntr prog_headers;
    Zeta_Core_SeqCntr sect_headers;
};

void Zeta_Core_ELF_Init(Zeta_Core_ELF* elf);

void Zeta_Core_ELF_Deinit(Zeta_Core_ELF* elf);

byte_t const* Zeta_Core_ELF_ReadHeader(Zeta_Core_ELF_Header* elf,
                                       byte_t const* src, size_t src_size);

bool_t Zeta_Core_ELF_ReadProgramHeaders(Zeta_Core_ELF* elf, byte_t const* src,
                                        size_t src_size);

bool_t Zeta_Core_ELF_ReadSectionHeaders(Zeta_Core_ELF* dst, byte_t const* src,
                                        size_t src_size);

bool_t Zeta_Core_ELF_CheckHeader(Zeta_Core_ELF_Header* header);

byte_t const* Zeta_Core_ELF_ReadProgramHeader(Zeta_Core_ELF_ProgramHeader* dst,
                                              Zeta_Core_ELF_Header* header,
                                              byte_t const* src,
                                              size_t src_size);

byte_t const* Zeta_Core_ELF_ReadSectionHeader(Zeta_Core_ELF_SectionHeader* dst,
                                              Zeta_Core_ELF_Header* header,
                                              byte_t const* src,
                                              size_t src_size);

void Zeta_Core_ELF_ReadSymbol(Zeta_Core_ELF_Symbol* dst,
                              Zeta_Core_ELF_Header* header, byte_t const* src,
                              size_t src_size);

ZETA_Core_ExternC_End;
