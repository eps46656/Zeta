#include <zeta/core/debugger.h>
#include <zeta/core/dynamic_vector.h>
#include <zeta/core/elf_utils.h>
#include <zeta/core/utils.h>

#define ASSERT_(cond, expression) \
    if (!(cond)) { expression }   \
    ZETA_Core_StaticAssert(TRUE)

#define Read_(src, length)                             \
    ({                                                 \
        ZETA_Core_AutoVar(ret, ReadFunc(src, length)); \
        src += length;                                 \
        ret;                                           \
    })

#define Write_(dst, val, length)     \
    {                                \
        WriteFunc(dst, val, length); \
        dst += length;               \
    }                                \
    ZETA_Core_StaticAssert(TRUE)

byte_t const* Zeta_Core_ELF_ReadHeader(Zeta_Core_ELF_Header* dst,
                                       byte_t const* src, size_t src_size) {
    ZETA_Core_DebugAssert(dst != NULL);

    ZETA_Core_DebugAssert(src != NULL);
    ZETA_Core_DebugAssert(52 <= src_size);

    u128_t (*ReadFunc)(byte_t const* src, unsigned length) =
        Zeta_Core_ReadLittleEndian;

    if (src[0] != 0x7f || src[1] != 'E' || src[2] != 'L' || src[3] != 'F') {
        return NULL;
    }

    src += 4;

    dst->ei_class = Read_(src, 1);
    if (dst->ei_class != 1 && dst->ei_class != 2) { return NULL; }

    unsigned length = dst->ei_class * 4;
    ZETA_Core_DebugAssert(length * 3 + 40 <= src_size);

    dst->ei_data = Read_(src, 1);
    if (dst->ei_data != 1 && dst->ei_data != 2) { return NULL; }

    if (dst->ei_data == 2) { ReadFunc = Zeta_Core_ReadBigEndian; }

    dst->ei_version = Read_(src, 1);
    dst->ei_osabi = Read_(src, 1);
    dst->ei_abiversion = Read_(src, 1);

    src += 7;

    dst->e_type = Read_(src, 2);
    dst->e_machine = Read_(src, 2);
    dst->e_version = Read_(src, 4);

    dst->e_entry = Read_(src, length);
    dst->e_phoff = Read_(src, length);
    dst->e_shoff = Read_(src, length);

    dst->e_flags = Read_(src, 4);

    dst->e_ehsize = Read_(src, 2);

    dst->e_phentsize = Read_(src, 2);
    dst->e_phnum = Read_(src, 2);

    dst->e_shentsize = Read_(src, 2);
    dst->e_shnum = Read_(src, 2);

    dst->e_shstrndx = Read_(src, 2);

    return src;
}

bool_t Zeta_Core_ELF_CheckHeader(Zeta_Core_ELF_Header* header) {
    ZETA_Core_DebugAssert(header != NULL);
    if (header->ei_class != 1 && header->ei_class != 2) { return FALSE; }
    if (header->ei_data != 1 && header->ei_data != 2) { return FALSE; }
    return TRUE;
}

byte_t* Zeta_Core_ELF_WriteHeader(byte_t* dst, size_t dst_size,
                                  Zeta_Core_ELF_Header* header) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(header != NULL);

    if (!Zeta_Core_ELF_CheckHeader(header)) { return NULL; }

    unsigned length = header->ei_class * 4;
    ZETA_Core_DebugAssert(dst_size <= length * 3 + 40);

    u128_t (*WriteFunc)(byte_t* dst, u128_t val, unsigned length) =
        header->ei_data == 1 ? Zeta_Core_WriteLittleEndian
                             : Zeta_Core_WriteBigEndian;

    Write_(dst, 0x7f, 1);
    Write_(dst, 'E', 1);
    Write_(dst, 'L', 1);
    Write_(dst, 'F', 1);

    Write_(dst, header->ei_class, 1);

    Write_(dst, header->ei_version, 1);
    Write_(dst, header->ei_osabi, 1);
    Write_(dst, header->ei_abiversion, 1);

    dst += 7;

    Write_(dst, header->e_type, 2);
    Write_(dst, header->e_machine, 2);
    Write_(dst, header->e_version, 4);

    Write_(dst, header->e_entry, length);
    Write_(dst, header->e_phoff, length);
    Write_(dst, header->e_shoff, length);

    Write_(dst, header->e_flags, 4);

    Write_(dst, header->e_ehsize, 2);

    Write_(dst, header->e_phentsize, 2);
    Write_(dst, header->e_phnum, 2);

    Write_(dst, header->e_shentsize, 2);
    Write_(dst, header->e_shnum, 2);

    Write_(dst, header->e_shstrndx, 2);

    return dst;
}

byte_t const* Zeta_Core_ELF_ReadProgramHeader(Zeta_Core_ELF_ProgramHeader* dst,
                                              Zeta_Core_ELF_Header* header,
                                              byte_t const* src,
                                              size_t src_size) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(header != NULL);
    ZETA_Core_DebugAssert(src != NULL);

    if (!Zeta_Core_ELF_CheckHeader(header)) { return NULL; }

    unsigned length = header->ei_class * 4;
    ZETA_Core_DebugAssert(src_size <= length * 6 + 8);

    u128_t (*ReadFunc)(byte_t const* src, unsigned length) =
        header->ei_data == 1 ? Zeta_Core_ReadLittleEndian
                             : Zeta_Core_ReadBigEndian;

    dst->p_type = Read_(src, 4);

    if (length == 8) { dst->p_flags = Read_(src, 4); }

    dst->p_offset = Read_(src, length);
    dst->p_vaddr = Read_(src, length);
    dst->p_paddr = Read_(src, length);
    dst->p_filesz = Read_(src, length);
    dst->p_memsz = Read_(src, length);

    if (length == 4) { dst->p_flags = Read_(src, 4); }

    dst->p_align = Read_(src, length);

    return src;
}

byte_t const* Zeta_Core_ELF_ReadSectionHeader(Zeta_Core_ELF_SectionHeader* dst,
                                              Zeta_Core_ELF_Header* header,
                                              byte_t const* src,
                                              size_t src_size) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(header != NULL);
    ZETA_Core_DebugAssert(src != NULL);

    if (!Zeta_Core_ELF_CheckHeader(header)) { return NULL; }

    unsigned length = header->ei_class * 4;
    ZETA_Core_DebugAssert(length * 6 + 16 <= src_size);

    u128_t (*ReadFunc)(byte_t const* src, unsigned length) =
        header->ei_data == 1 ? Zeta_Core_ReadLittleEndian
                             : Zeta_Core_ReadBigEndian;

    dst->sh_name = Read_(src, 4);
    dst->sh_type = Read_(src, 4);

    dst->sh_flags = Read_(src, length);

    dst->sh_addr = Read_(src, length);
    dst->sh_offset = Read_(src, length);
    dst->sh_size = Read_(src, length);

    dst->sh_link = Read_(src, 4);
    dst->sh_info = Read_(src, 4);

    dst->sh_addralign = Read_(src, length);
    dst->sh_entsize = Read_(src, length);

    return src;
}

void Zeta_Core_ELF_ReadSymbol(Zeta_Core_ELF_Symbol* dst,
                              Zeta_Core_ELF_Header* header, byte_t const* src,
                              size_t src_size) {
    ZETA_Core_DebugAssert(dst != NULL);
    ZETA_Core_DebugAssert(header != NULL);
    ZETA_Core_DebugAssert(src != NULL);

    unsigned length = header->ei_class * 4;
    ZETA_Core_DebugAssert(length * 4 + 2 <= src_size);

    u128_t (*ReadFunc)(byte_t const* src, unsigned length) =
        header->ei_data == 1 ? Zeta_Core_ReadLittleEndian
                             : Zeta_Core_ReadBigEndian;

    dst->st_name = Read_(src, length);

    if (length == 4) {
        dst->st_value = Read_(src, length);
        dst->st_size = Read_(src, length);
    }

    dst->st_info = Read_(src, 1);
    dst->st_other = Read_(src, 1);
    dst->st_shndx = Read_(src, length);

    if (length == 8) {
        dst->st_value = Read_(src, length);
        dst->st_size = Read_(src, length);
    }
}

bool_t Zeta_Core_ELF_ReadProgramHeaders(Zeta_Core_ELF* elf, byte_t const* src,
                                        size_t src_size) {
    ZETA_Core_DebugAssert(elf != NULL);
    ZETA_Core_DebugAssert(src != NULL);

    if (!Zeta_Core_ELF_CheckHeader(&elf->header)) { return FALSE; }

    size_t prog_headers_cnt = elf->header.e_phnum;

    if (src_size <
        elf->header.e_phoff + elf->header.e_phentsize * prog_headers_cnt) {
        return FALSE;
    }

    if (elf->prog_headers.context == NULL) {
        Zeta_Core_DynamicVector* dv = ZETA_Core_Allocator_SafeAllocate(
            &Zeta_Core_DynamicVector_seq_cntr_vtable, alignof(max_align_t),
            sizeof(Zeta_Core_DynamicVector));

        dv->width = sizeof(Zeta_Core_ELF_ProgramHeader);
        dv->width = sizeof(Zeta_Core_ELF_ProgramHeader);
        dv->seg_capacity = 8;

        dv->data_allocator = &Zeta_Core_DynamicVector_seq_cntr_vtable;
        dv->seg_allocator = &Zeta_Core_DynamicVector_seq_cntr_vtable;

        Zeta_Core_DynamicVector_Init(dv);
    }

    size_t cur_prog_header_cnt = ZETA_Core_SeqCntr_GetSize(elf->prog_headers);

    if (cur_prog_header_cnt < prog_headers_cnt) {
        ZETA_Core_SeqCntr_PushR(elf->prog_headers,
                                prog_headers_cnt - cur_prog_header_cnt, NULL);
    } else {
        ZETA_Core_SeqCntr_PopR(elf->prog_headers,
                               cur_prog_header_cnt - prog_headers_cnt);
    }

    Zeta_Core_ELF_ProgramHeader tmp;

    void* cursor = ZETA_Core_SeqCntr_AllocaCursor(elf->prog_headers);
    ZETA_Core_SeqCntr_Access(elf->prog_headers, 1, cursor, NULL);

    for (size_t prog_header_i = 0; prog_header_i < prog_headers_cnt;
         ++prog_header_i) {
        byte_t const* v = Zeta_Core_ELF_ReadProgramHeader(
            &tmp, &elf->header,
            src + elf->header.e_phoff + elf->header.e_phentsize * prog_header_i,
            elf->header.e_phentsize);

        if (v == NULL) { return FALSE; }

        ZETA_Core_SeqCntr_Write(elf->prog_headers, cursor, 1, &tmp, cursor);
    }

    return TRUE;
}

bool_t Zeta_Core_ELF_ReadSesctionHeaders(Zeta_Core_ELF* elf, byte_t const* src,
                                         size_t src_size) {
    ZETA_Core_DebugAssert(elf != NULL);
    ZETA_Core_DebugAssert(src != NULL);

    if (!Zeta_Core_ELF_CheckHeader(&elf->header)) { return FALSE; }

    size_t sect_headers_cnt = elf->header.e_phnum;

    if (src_size <
        elf->header.e_shoff + elf->header.e_shentsize * sect_headers_cnt) {
        return FALSE;
    }

    if (elf->sect_headers.context == NULL) {
        Zeta_Core_DynamicVector* dv = ZETA_Core_Allocator_SafeAllocate(
            &Zeta_Core_DynamicVector_seq_cntr_vtable, alignof(max_align_t),
            sizeof(Zeta_Core_DynamicVector));

        dv->width = sizeof(Zeta_Core_ELF_SectionHeader);
        dv->width = sizeof(Zeta_Core_ELF_SectionHeader);
        dv->seg_capacity = 8;

        dv->data_allocator = &Zeta_Core_DynamicVector_seq_cntr_vtable;
        dv->seg_allocator = &Zeta_Core_DynamicVector_seq_cntr_vtable;

        Zeta_Core_DynamicVector_Init(dv);
    }

    size_t cur_sect_header_cnt = ZETA_Core_SeqCntr_GetSize(elf->sect_headers);

    if (cur_sect_header_cnt < sect_headers_cnt) {
        ZETA_Core_SeqCntr_PushR(elf->sect_headers,
                                sect_headers_cnt - cur_sect_header_cnt, NULL);
    } else {
        ZETA_Core_SeqCntr_PopR(elf->sect_headers,
                               cur_sect_header_cnt - sect_headers_cnt);
    }

    Zeta_Core_ELF_SectionHeader tmp;

    void* cursor = ZETA_Core_SeqCntr_AllocaCursor(elf->sect_headers);
    ZETA_Core_SeqCntr_Access(elf->sect_headers, 1, cursor, NULL);

    for (size_t sect_header_i = 0; sect_header_i < sect_headers_cnt;
         ++sect_header_i) {
        byte_t const* v = Zeta_Core_ELF_ReadSectionHeader(
            &tmp, &elf->header,
            src + elf->header.e_shoff + elf->header.e_shentsize * sect_header_i,
            elf->header.e_phentsize);

        if (v == NULL) { return FALSE; }

        ZETA_Core_SeqCntr_Write(elf->sect_headers, cursor, 1, &tmp, cursor);
    }

    return TRUE;
}
