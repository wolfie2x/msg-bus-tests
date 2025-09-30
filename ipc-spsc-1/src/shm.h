#ifndef ULTRAIPC_SHM_HPP
#define ULTRAIPC_SHM_HPP
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <cstring>
#include "ring.h"


namespace ultraipc {


// Payload for ping/pong
struct Msg {
uint64_t seq;
uint64_t t0_ns; // timestamp by sender (ping)
uint64_t pad; // keep 24B payload; align to 32 if desired
};


// Shared memory region layout: two rings for request and response
// Capacity is compile-time constant for simplicity
template <uint64_t CAPACITY>
struct SharedRegion {
SpscRing<Msg, CAPACITY> req; // ping -> pong
SpscRing<Msg, CAPACITY> rsp; // pong -> ping
};


// RAII mapper for POSIX shared memory objects
class Shm {
int fd_ = -1;
size_t sz_ = 0;
void* addr_ = nullptr;
public:
Shm(const std::string& name, size_t sz, bool create) : sz_(sz) {
int flags = O_RDWR;
if (create) flags |= O_CREAT;
fd_ = ::shm_open(name.c_str(), flags, 0666);
if (fd_ < 0) throw std::runtime_error("shm_open failed: " + std::string(strerror(errno)));
if (create) {
if (ftruncate(fd_, sz_) != 0) throw std::runtime_error("ftruncate failed");
}
addr_ = ::mmap(nullptr, sz_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
if (addr_ == MAP_FAILED) throw std::runtime_error("mmap failed");
}
~Shm() {
if (addr_ && addr_ != MAP_FAILED) ::munmap(addr_, sz_);
if (fd_ >= 0) ::close(fd_);
}
void* addr() const { return addr_; }
};


} // namespace ultraipc


#endif // ULTRAIPC_SHM_HPP