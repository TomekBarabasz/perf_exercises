#pragma once
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

namespace utils
{
struct MemMappedFile {
    const char* data;
    size_t size;
    MemMappedFile(const char* filename)
    {
        fd = open(filename,O_RDONLY);
        if (-1==fd) {
            throw std::invalid_argument("open file error");
        }
        size = lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);
        data_ = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (MAP_FAILED == data_) {
            throw std::runtime_error("failed to map input file to memory");
        }
        data = static_cast<const char*>( data_ );
    }
    ~MemMappedFile() 
    {
        munmap(data_,size);
        close(fd);
        data = 0;
        size = 0;
        fd = 0;
    }
protected:
    int fd;
    void* data_;
};
}