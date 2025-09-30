#include "subscriber.h"
#include <cstdio>


#include "subscriber.h"
#include "stat.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <chrono>


Subscriber::Subscriber(const char* mcast_addr, uint16_t port)
    : stats_(1000000) // default max_msgs
{
    sock_ = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    int reuse = 1;
    setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock_, (sockaddr*)&addr_, sizeof(addr_));
    ip_mreq mreq = {};
    inet_pton(AF_INET, mcast_addr, &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    setsockopt(sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

Subscriber::~Subscriber() {
    if (sock_ != -1) close(sock_);
}

ssize_t Subscriber::recv(void* payload_buf, size_t buf_size, MsgHeader& header) {
    alignas(8) char buf[1500];
    ssize_t n = ::recv(sock_, buf, sizeof(buf), 0);
    if (n < (ssize_t)sizeof(MsgHeader)) return -1;
    MsgHeader* hdr = reinterpret_cast<MsgHeader*>(buf);
    header = *hdr;
    size_t payload_len = n - sizeof(MsgHeader);
    if (payload_len > buf_size) return -1;
    memcpy(payload_buf, buf + sizeof(MsgHeader), payload_len);
    return payload_len;
}


void Subscriber::on_data(const void* data, size_t len, uint64_t mc_seq) {
    int64_t msg_val = 0;
    memcpy(&msg_val, data, sizeof(int64_t));
    printf("Subscriber::on_data: mc_seq=%llu, payload_len=%zu, cmd=%lld, app_seq=%lld\n", 
        mc_seq, len, msg_val, msg_val);
    if (payload_num == CMD_END) {
        // End of test detected
        stats_.write_csv(csv_file);
        printf("End of test detected. Latencies written to %s\n", csv_file);
        break;
    }
    // Calculate latency
    int64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    int64_t latency = now_ns - header.timestamp_ns;
    if (payload_num !=  CMD_WARMUP ) {
        stats_.record(header.seq, latency);
    }
    if (DBG >= 2) {
        int is_warmup = (payload_num == CMD_WARMUP ? 1 : 0);
        printf("seq=%lu ts=%lu latency=%ld ns payload=%zd bytes; warmup=%d\n", 
            header.seq, header.timestamp_ns, latency, n, is_warmup);
    }
}

// Implementation of Subscriber::run
void Subscriber::run(const char* csv_file) {
    alignas(8) char payload[1500 - sizeof(MsgHeader)];
    MsgHeader header;
    size_t received = 0;

    while (true) {
        ssize_t n = recv(payload, sizeof(payload), header);
        if (n < 0) continue;
        int64_t payload_num = 0;
        memcpy(&payload_num, payload, sizeof(int64_t));
        received++;
        if (payload_num == CMD_END) {
            // End of test detected
            stats_.write_csv(csv_file);
            printf("End of test detected. Latencies written to %s\n", csv_file);
            break;
        }
        // Calculate latency
        int64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        int64_t latency = now_ns - header.timestamp_ns;
        if (payload_num !=  CMD_WARMUP ) {
            stats_.record(header.seq, latency);
        }
        if (DBG >= 2) {
            int is_warmup = (payload_num == CMD_WARMUP ? 1 : 0);
            printf("seq=%lu ts=%lu latency=%ld ns payload=%zd bytes; warmup=%d\n", 
                header.seq, header.timestamp_ns, latency, n, is_warmup);
        }
    }
}



#include "subscriber.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error.. Usage: %s <mcast_addr> <port>\n", argv[0]);
        return 1;
    }
    printf("Subscriber starting ...\n");

    const char* mcast_addr = argv[1];
    uint16_t port = atoi(argv[2]);

    Subscriber sub(mcast_addr, port);
    sub.run();
    return 0;
}
