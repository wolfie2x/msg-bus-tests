
#include "publisher.h"
#include <cstring>
#include <chrono>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "MCTransport.h"

Publisher::Publisher(const char* mcast_addr, uint16_t port) {
    transport_ = new MCTransport(mcast_addr, port);
}

Publisher::~Publisher() {
    delete transport_;
}

bool Publisher::send(size_t len, int msg_cmd  /*= CMD_NONE*/) {
    void* buf = transport_->get_send_buffer(len);
    if (!buf) return false;
    app_seq_++;    
    int64_t msg_val = (msg_cmd == CMD_NONE) ? app_seq_ : msg_cmd;
    if (len < sizeof(msg_val)) return false;
    std::memcpy(buf, &msg_val, sizeof(msg_val));
    return transport_->send(len);
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Error.. Usage: %s <mcast_addr> <port> <msg-size> <msg-rate> <duration-sec>\n", argv[0]);
        return 1;
    }
    printf("Publisher starting ...\n");

    const char* mcast_addr = argv[1];
    uint16_t port = atoi(argv[2]);
    size_t msg_size = atoi(argv[3]);
    int msg_rate = atoi(argv[4]); // messages per second
    int duration_sec = atoi(argv[5]); // seconds
    if (msg_size < 1 || msg_size > 1400) {
        fprintf(stderr, "msg-size must be between 1 and 1400\n");
        return 1;
    }
    if (msg_rate < 1 || duration_sec < 1) {
        fprintf(stderr, "msg-rate and test-duration must be >= 1\n");
        return 1;
    }

    int msg_count = msg_rate * duration_sec;
    int warmup_count = std::min(int(msg_count*0.01), 1000);
    printf("Publisher: msg_size=%zu msg_rate=%d duration=%d sec total_msgs=%d warmup_msgs=%d\n",
           msg_size, msg_rate, duration_sec, msg_count, warmup_count);
    
    Publisher pub(mcast_addr, port);
    using clock = std::chrono::steady_clock;
    auto interval = std::chrono::microseconds(1000000 / msg_rate);
    auto next_send = clock::now();
    for (int i = 0; i < msg_count; ++i) {
        int msg_cmd = (i < warmup_count ? CMD_WARMUP : CMD_NONE);
        // printf("Publisher: cmd=%d\n", msg_cmd);
        bool result = pub.send(msg_size, msg_cmd);
        if (!result) {
            fprintf(stderr, "send failed\n");
        }
        next_send += interval;
        while (clock::now() < next_send) {
            // busy wait
        }
    }
    // Send end-of-test marker
    pub.send(msg_size, CMD_END);
    return 0;
}
