#include "MCTransport.h"
#include <cstdio>

MCTransport::MCTransport(const char* group_addr, int port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(group_addr);
    // For multicast receive, join group (not used in send-only)
    // For send, just set up socket and addr
    // Buffer is statically sized, nothing else needed
}

MCTransport::~MCTransport() {
    if (sockfd >= 0) close(sockfd);
}

bool MCTransport::send(size_t len) {
    if (len + sizeof(MCHeader) > sizeof(buf_)) return false;
    MCHeader* hdr = reinterpret_cast<MCHeader*>(buf_);
    hdr->mc_seq = ++mc_send_seq_;
    hdr->payload_len = static_cast<uint32_t>(len);
    size_t total = sizeof(MCHeader) + len;
    ssize_t sent = sendto(sockfd, buf_, total, 0, (sockaddr*)&addr, sizeof(addr));
    return sent == (ssize_t)total;
}


int MCTransport::run_recv_loop(RecvMode mode) {
    socklen_t addrlen = sizeof(addr);
    int flags = 0;
    if (mode == RecvMode::Polling)
        flags |= MSG_DONTWAIT;

    // Polling mode: loop until full message is received
    while (true) {
        ssize_t n = recvfrom(sockfd, buf_, sizeof(buf_), flags, (sockaddr*)&addr, &addrlen);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) // No data available, continue polling
                continue;
            else            // Actual error
                return -1;            
        }
        if (n < (ssize_t)sizeof(MCHeader)) {  // incomplete header
            printf("ERR: Incomplete header received: %zd bytes\n", n);
            continue; 
        }
        MCHeader* hdr = reinterpret_cast<MCHeader*>(buf_);
        size_t payload_len = hdr->payload_len;
        if (hdr->mc_seq <= mc_recv_seq_) {
            // Duplicate or out-of-order message, ignore
            printf("ERR: Duplicate or out-of-order message received: %lu\n", hdr->mc_seq);
            continue;
        } else if (hdr->mc_seq > mc_recv_seq_ + 1) {
            // Missed messages
            printf("WARN: Missed messages. Last seq=%lu, received seq=%lu\n", mc_recv_seq_, hdr->mc_seq);
        }
        if (n < (ssize_t)(sizeof(MCHeader) + payload_len)) {
            printf("ERR: Incomplete message received: %zd bytes\n", n);
            continue;
        }
        if (callback_)
            callback_->on_data(buf_ + sizeof(MCHeader), payload_len, hdr->mc_seq);
        
        mc_recv_seq_ = hdr->mc_seq;
    }
}


void MCTransport::set_callback(ITransportCB* cb) {
    callback_ = cb;
}

void* MCTransport::get_send_buffer(size_t len) {
    // Leave room for header in buf_
    if (len + sizeof(MCHeader) > sizeof(buf_)) return nullptr;
    return buf_ + sizeof(MCHeader);
}
