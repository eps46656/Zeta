#include <file_sys_fat32.h>

#include <fstream>

struct FileDisk {
    std::fstream fs;
};

void FileDisk_Open(void* context, char const* path) {
    FileDisk* fd = (FileDisk*)context;

    fd->fs.open(path, std::ios::binary | std::ios::in | std::ios::out);
}

size_t FileDisk_GetSize(void* context) {
    FileDisk* fd = (FileDisk*)context;

    if (fd->fs) { return 0; }

    fd->fs.seekg(0, std::fstream::end);
    return fd->fs.tellg();
}

void FileDisk_Read(void* context, size_t beg, size_t end, byte_t* dst) {
    FileDisk* fd = (FileDisk*)context;

    fd->fs.seekg(beg, std::fstream::beg);

    for (byte_t const* dst_end = dst + (end - beg); dst < dst_end; ++dst) {
        *dst = fd->fs.get();
    }
}

void FileDisk_Write(void* context, size_t beg, size_t end, byte_t const* src) {
    FileDisk* fd = (FileDisk*)context;

    fd->fs.seekp(beg, std::fstream::beg);

    for (byte_t const* src_end = src + (end - beg); src < src_end; ++src) {
        fd->fs.put(*src);
    }
}

void main1() {
    FileDisk fd;
    FileDisk_Open(&fd, "");

    Zeta_Core_Disk disk;
    disk.context = &fd;
    disk.GetSize = FileDisk_GetSize;
    disk.Read = FileDisk_Read;
    disk.Write = FileDisk_Write;

    //
}

int main() {
    main1();
    return 0;
}
