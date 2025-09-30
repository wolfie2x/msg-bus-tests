#include "sequencer.h"
#include "MCTransport.h"

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <pthread.h>
#include <cstring>


Sequencer::Sequencer(ITransport* transport) : transport_(transport) {
    transport_->set_callback(this);
}

void Sequencer::on_data(int64_t sender_id, const void* data, size_t len, int64_t mc_seq) {
    
    AppMsgData* msg = nullptr;

    if (SEQ_ZERO_COPY) {
        // Zero-copy mode: data is already in transport's buffer, just modify in place
        msg = (AppMsgData*)data;
    
    } else {
        // Get buffer for outgoing message
        void* buf = transport_->get_send_buffer(len);
        if (!buf) {
            fprintf(stderr, "Failed to get send buffer\n");
            return;
        }

        // Copy incoming data to send buffer
        memcpy(buf, data, len);

        // Cast to AppMsgData for timestamp modification
        msg = static_cast<AppMsgData*>(buf);
    }

    
    // Timestamp in ts1_ns before sending out
    msg->ts1_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
    
    msg->tx_id = ++tx_seq_;

    if (DBG >= 2) {
        printf("Sequencer::on_data: sender_id=%ld, mc_seq=%ld, app_seq=%ld, tx_id=%ld, ts1_ns=%ld\n",
               sender_id, mc_seq, msg->app_seq, msg->tx_id, msg->ts1_ns);
    }

    // Forward the message
    if (!transport_->send(len)) {
        fprintf(stderr, "Failed to send message\n");
    }
}


int main(int argc, char* argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Error.. Usage: %s <core> <recv_mcast_addr> <recv_port> <send_mcast_addr> <send_port>\n", argv[0]);
        return 1;
    }
    printf("Sequencer starting ...\n");

    int64_t core = atoll(argv[1]);
    // Pin this thread to the specified core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
    } else {
        printf("Sequencer: pinned to core %ld\n", core);
    }

    const char* recv_mcast_addr = argv[2];
    uint16_t recv_port = atoi(argv[3]);
    const char* send_mcast_addr = argv[4];
    uint16_t send_port = atoi(argv[5]);

    printf("Receiving from multicast group %s:%d\n", recv_mcast_addr, recv_port);
    printf("Sending to multicast group %s:%d\n", send_mcast_addr, send_port);

    MCTransport transport;
    
    // Initialize for receiving
    if (!transport.init_recv(recv_mcast_addr, recv_port)) {
        fprintf(stderr, "Failed to initialize receive transport\n");
        return 1;
    }
    
    // Initialize for sending (using separate multicast address)
    int64_t sender_id = 2;  // Default sender ID for sequencer
    if (!transport.init_send(sender_id, send_mcast_addr, send_port)) {
        fprintf(stderr, "Failed to initialize send transport\n");
        return 1;
    }

    Sequencer sequencer(&transport);

    // Run receive loop in polling mode for lowest latency
    transport.run_recv_loop(RecvMode::Polling);

    return 0;
}
