#include "subscriber.h"
#include "MCTransport.h"
#include "stat.h"

#include <cstdio>
#include <cstdlib>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <chrono>


Subscriber::Subscriber(Stat* stats, ITransport* transport) 
    : stats_(stats), transport_(transport) {
    transport_->set_callback(this);
}

void Subscriber::on_data(const void* data, size_t len, uint64_t mc_seq) {
    // Use aligned struct for efficient access
    const AppMsgData* msg = static_cast<const AppMsgData*>(data);
    
    if (msg->msg_cmd == CMD_END) {
        printf("End of test detected.\n");
        stats_->write_csv();
        exit(0);
    }
    // Calculate latency
    int64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    int64_t latency = now_ns - msg->ts0_ns;
    if (msg->msg_cmd !=  CMD_WARMUP ) {
        stats_->record(mc_seq, latency);
    }
    if (DBG >= 2) {
        printf("Subscriber::on_data: mc_seq=%ld, payload_len=%ld, msg_cmd=%ld, app_seq=%ld, latency=%ld\n", 
            mc_seq, len, (int64_t)msg->msg_cmd, (int64_t)msg->app_seq, latency);
    }
}



int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error.. Usage: %s <mcast_addr> <port>\n", argv[0]);
        return 1;
    }
    printf("Subscriber starting ...\n");
    Stat stats(10000, "latency.csv");
    
    const char* mcast_addr = argv[1];
    uint16_t port = atoi(argv[2]);
    printf("Joining multicast group %s:%d\n", mcast_addr, port);

    MCTransport mc_transport(mcast_addr, port);

    Subscriber sub(&stats, &mc_transport);
    
    mc_transport.run_recv_loop(RecvMode::Blocking);

    return 0;
}
