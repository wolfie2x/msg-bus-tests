#ifndef MCTRANSPORT_H
#define MCTRANSPORT_H

#include "ITransport.h"
#include "common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>


class MCTransport : public ITransport {
public:
    MCTransport(const char* group_addr, int port);
    ~MCTransport();
    void* get_send_buffer(size_t len) override;
    bool send(size_t len) override;
    int run_recv_loop(RecvMode mode) override;
    void set_callback(ITransportCB* cb) override;
private:
    int sockfd;
    sockaddr_in addr;
    alignas(8) char buf_[1500];
    ITransportCB* callback_ = nullptr;
    uint64_t mc_send_seq_ = 0;
    uint64_t mc_recv_seq_ = 0;  // last received seq
    struct MsgHeader {
        uint64_t mc_seq;        // multicast seq
        uint32_t payload_len;   // payload length in bytes
    };
};

#endif // MCTRANSPORT_H
