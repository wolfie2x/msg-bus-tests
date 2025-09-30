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
    MCTransport();
    ~MCTransport();

    // for sender
    bool init_send(int64_t sender_id, const char* group_addr, int port);
    void* get_send_buffer(size_t len) override;
    bool send(size_t len) override;
    
    // for receiver
    bool init_recv(const char* group_addr, int port);
    int run_recv_loop(RecvMode mode) override;
    void set_callback(ITransportCB* cb) override;

private:
    // Send socket and address
    int send_sockfd;
    sockaddr_in send_addr;
    
    // Receive socket and address
    int recv_sockfd;
    sockaddr_in recv_addr;
    
    alignas(8) char buf_[1500];
    ITransportCB* callback_ = nullptr;
    int64_t sender_id_ = 0;
    int64_t mc_send_seq_ = 0;
    int64_t mc_recv_seq_[MAX_SENDERS] = {0};  // last received seq per sender

    struct alignas(8) MCHeader {
        int64_t sender_id;     // sender identifier
        int64_t mc_seq;        // multicast seq
        int64_t payload_len;   // payload length in bytes
    };
};

#endif // MCTRANSPORT_H
