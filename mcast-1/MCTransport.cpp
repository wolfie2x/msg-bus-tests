#include "MCTransport.h"
#include <cstdio>
#include <netinet/ip.h>
#include <sys/socket.h>

// Socket option configuration structure
struct SocketOpt {
    int level;
    int option;
    const void* value;
    size_t value_len;
    bool critical;  // If false, failure won't abort initialization
    const char* name;  // For error reporting
};

// Helper function to apply socket options from array
static bool apply_socket_options(int sockfd, const SocketOpt* opts, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (setsockopt(sockfd, opts[i].level, opts[i].option, opts[i].value, opts[i].value_len) < 0) {
            if (opts[i].critical) {
                close(sockfd);
                return false;
            } else {
                printf("Warning: %s not supported, continuing without it.\n", opts[i].name);
            }
        }
    }
    return true;
}

MCTransport::MCTransport() : send_sockfd(-1), recv_sockfd(-1) {
    memset(&send_addr, 0, sizeof(send_addr));
    memset(&recv_addr, 0, sizeof(recv_addr));
}

MCTransport::~MCTransport() {
    if (send_sockfd >= 0) close(send_sockfd);
    if (recv_sockfd >= 0) close(recv_sockfd);
}


bool MCTransport::init_send(int64_t sender_id, const char* group_addr, int port) {
    sender_id_ = sender_id;
    send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_sockfd < 0) return false;
    
    // Ultra low latency socket options
    int sndbuf = 8192;
    int tos = IPTOS_LOWDELAY;
    int ttl = 1;
    static const SocketOpt send_opts[] = {
        {SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf), true, "SO_SNDBUF"},              // Small buffer for low latency
        {IPPROTO_IP, IP_TOS, &tos, sizeof(tos), true, "IP_TOS"},                          // Low delay priority
        {IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl), true, "IP_MULTICAST_TTL"}       // Local network only
    };

    if (!apply_socket_options(send_sockfd, send_opts, sizeof(send_opts)/sizeof(send_opts[0]))) {
        send_sockfd = -1;
        return false;
    }

    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(port);
    send_addr.sin_addr.s_addr = inet_addr(group_addr);

    return true;
}

bool MCTransport::init_recv(const char* group_addr, int port) {
    recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_sockfd < 0) return false;
    
    // Ultra low latency socket options
    int reuse = 1;
    int rcvbuf = 8192;
    int busy_poll = 100;     // in microseconds
    int priority = 6;
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(group_addr);
    mreq.imr_interface.s_addr = INADDR_ANY;

    static const SocketOpt recv_opts[] = {
        {SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse), true, "SO_REUSEADDR"},
        {SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf), true, "SO_RCVBUF"},
        {SOL_SOCKET, SO_BUSY_POLL, &busy_poll, sizeof(busy_poll), false, "SO_BUSY_POLL"},
        {SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority), true, "SO_PRIORITY"},
        {IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq), true, "IP_ADD_MEMBERSHIP"}
    };

    if (!apply_socket_options(recv_sockfd, recv_opts, sizeof(recv_opts)/sizeof(recv_opts[0]))) {
        recv_sockfd = -1;
        return false;
    }

    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(port);
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(recv_sockfd, (sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
        close(recv_sockfd);
        recv_sockfd = -1;
        return false;
    }

    return true;
}

bool MCTransport::send(size_t len) {
    if (len + sizeof(MCHeader) > sizeof(buf_)) return false;
    MCHeader* hdr = reinterpret_cast<MCHeader*>(buf_);
    hdr->sender_id = sender_id_;
    hdr->mc_seq = ++mc_send_seq_;
    hdr->payload_len = static_cast<int64_t>(len);
    size_t total = sizeof(MCHeader) + len;
    ssize_t sent = sendto(send_sockfd, buf_, total, 0, (sockaddr*)&send_addr, sizeof(send_addr));
    return sent == (ssize_t)total;
}


int MCTransport::run_recv_loop(RecvMode mode) {
    socklen_t addrlen = sizeof(recv_addr);
    int flags = 0;
    if (mode == RecvMode::Polling)
        flags |= MSG_DONTWAIT;

    // Polling mode: loop until full message is received
    while (true) {
        ssize_t n = recvfrom(recv_sockfd, buf_, sizeof(buf_), flags, (sockaddr*)&recv_addr, &addrlen);
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
        
        auto sender = hdr->sender_id;
        
        // Validate sender_id bounds
        if (sender < 0 || sender >= MAX_SENDERS) {
            printf("ERR: Invalid sender_id: %ld\n", sender);
            continue;
        }
        
        auto last_seq = mc_recv_seq_[sender];
        
        if (hdr->mc_seq == 1) {
            printf("Resetting mc_seq to 0: sender=%ld\n", sender);
            mc_recv_seq_[sender] = 0;
        } else if (hdr->mc_seq <= last_seq) {
            // Duplicate or out-of-order message, ignore
            printf("ERR: Duplicate or out-of-order message: sender=%ld: received seq=%ld, last seq=%ld\n", 
                    sender, hdr->mc_seq, last_seq);
            continue;
        } else if (hdr->mc_seq > last_seq + 1) {
            // Missed messages
            printf("WARN: Missed messages: sender=%ld. Last seq=%ld, received seq=%ld\n", 
                    sender, last_seq, hdr->mc_seq);
        }
    
        if (n < (ssize_t)(sizeof(MCHeader) + hdr->payload_len)) {
            printf("ERR: Incomplete message received: %zd bytes\n", n);
            continue;
        }

        mc_recv_seq_[sender] = hdr->mc_seq;
        
        if (callback_)
            callback_->on_data(sender, buf_ + sizeof(MCHeader), hdr->payload_len, hdr->mc_seq);
    
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
