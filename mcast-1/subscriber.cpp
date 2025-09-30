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
#include <pthread.h>


Subscriber::Subscriber(Stat *stats, ITransport *transport)
    : stats_(stats), transport_(transport)
{
    transport_->set_callback(this);
}


void Subscriber::check_end()
{
    bool is_active = false;
    for (int i = 0; i < MAX_SENDERS; i++) {
        if (senders[i] != 0) {
            is_active = true;
            break;
        }
    }
    if (!is_active) {
        stats_->write_csv();
        exit(0);
    }
}


void Subscriber::on_data(int64_t sender_id, const void *data, size_t len, int64_t mc_seq)
{
    // Use aligned struct for efficient access
    const AppMsgData *msg = static_cast<const AppMsgData *>(data);

    if (senders[sender_id] == 0) {
        senders[sender_id] = 1;
        printf("Subscriber: detected new sender: id=%ld\n", sender_id);
    }

    if (msg->msg_cmd == CMD_END)
    {
        printf("End of test detected: sender=%ld\n", sender_id);
        senders[sender_id] = 0;
        check_end();
    }

    // Calculate latency
    int64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count();
    int64_t latency = now_ns - msg->ts0_ns;
    if (msg->msg_cmd != CMD_WARMUP)
    {
        stats_->record(sender_id, mc_seq, latency);
    }
    if (DBG >= 2)
    {
        printf("Subscriber::on_data: sender_id=%ld, mc_seq=%ld, payload_len=%ld, msg_cmd=%ld, app_seq=%ld, latency=%ld\n",
               sender_id, mc_seq, len, (int64_t)msg->msg_cmd, (int64_t)msg->app_seq, latency);
    }
}


int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Error.. Usage: %s <core> <mcast_addr> <port>\n", argv[0]);
        return 1;
    }
    printf("Subscriber starting ...\n");

    int64_t core = atoll(argv[1]);
    // Pin this thread to the specified core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
    }else {
        printf("Subscriber: pinned to core %ld\n", core);
    }

    const char *mcast_addr = argv[2];
    uint16_t port = atoi(argv[3]);
    printf("Joining multicast group %s:%d\n", mcast_addr, port);

    MCTransport mc_transport;
    if (!mc_transport.init_recv(mcast_addr, port))
    {
        fprintf(stderr, "Failed to initialize receive transport\n");
        return 1;
    }

    Stat *stats = new Stat(10000000, "latency.csv");

    Subscriber sub(stats, &mc_transport);

    mc_transport.run_recv_loop(RecvMode::Polling);

    delete stats;
    return 0;
}
