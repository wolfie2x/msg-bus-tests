
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


Publisher::Publisher(ITransport* transport) : 
    transport_(transport) {
}

Publisher::~Publisher() {
    delete transport_;
}

void Publisher::Init(int msg_size, int msg_rate, int msg_count) {
    msg_size_ = msg_size;
    msg_rate_ = msg_rate;
    msg_count_ = msg_count;

    if ((size_t)msg_size_ < sizeof(AppMsgData)) {
        printf("Error: msg_size %d too small, must be at least %zu\n", 
               msg_size_, sizeof(AppMsgData));
        exit(1);
    }
}

bool Publisher::send(int64_t ts_ns, int64_t msg_cmd  /*= CMD_NONE*/) {
    
    void* buf = transport_->get_send_buffer(msg_size_);
    if (!buf) return false;
    app_seq_++;    
    
    // Use aligned struct for efficient access
    AppMsgData* msg = static_cast<AppMsgData*>(buf);
    msg->app_seq = app_seq_;
    msg->msg_cmd = msg_cmd;
    msg->ts0_ns = ts_ns;
    msg->ts1_ns = 0;  // unused for now
    
    if (DBG >= 2) {
        printf("Publisher::send: app_seq=%ld, msg_cmd=%ld, ts0_ns=%ld\n", 
               (int64_t)msg->app_seq, (int64_t)msg->msg_cmd, (int64_t)msg->ts0_ns);
    }
    return transport_->send(msg_size_);
}


void Publisher::start_publishing() {
    int warmup_count = std::min(int(msg_count_*0.01), 1000);
    printf("Publisher: msg_size=%d, msg_rate=%d, total_msgs=%d, warmup_msgs=%d\n",
           msg_size_, msg_rate_, msg_count_, warmup_count);
    
    using clock = std::chrono::steady_clock;
    auto interval = std::chrono::microseconds(1000000 / msg_rate_);
    int64_t curr_ts_ns = 0;
    auto curr_time = clock::now();
    auto next_send = curr_time;
    for (int i = 0; i < msg_count_; ++i) {
        int msg_cmd = (i < warmup_count ? CMD_WARMUP : CMD_NONE);
        // printf("Publisher: cmd=%d\n", msg_cmd);
        curr_ts_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            curr_time.time_since_epoch()).count();

        bool result = send(curr_ts_ns, msg_cmd);
        if (!result) {
            fprintf(stderr, "send failed\n");
        }
        next_send += interval;
        while (true) {  // busy wait
            curr_time = clock::now();
            if (curr_time > next_send) break;
        }
    }
    // Send end-of-test marker
    send(curr_ts_ns, CMD_END);
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
    printf("msg_size=%zu, msg_rate=%d, duration=%ds, total_msgs=%d\n",
           msg_size, msg_rate, duration_sec, msg_count);
    
    MCTransport* transport = new MCTransport(mcast_addr, port);
    
    Publisher pub(transport);
    pub.Init(msg_size, msg_rate, msg_count);
    pub.start_publishing();
    
    return 0;
}
