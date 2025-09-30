#pragma once
#include "common.h"
#include "stat.h"
#include <netinet/in.h>

class Subscriber {
public:
    Subscriber(const char* mcast_addr, uint16_t port);
    ~Subscriber();
    // Receives a message, returns payload length or -1 on error
    ssize_t recv(void* payload_buf, size_t buf_size, MsgHeader& header);
    void run(const char* csv_file = "latency.csv");

    // ITransportCB implementation
    void on_data(const void* data, size_t len, uint64_t mc_seq);
private:
    int sock_ = -1;
    sockaddr_in addr_{};
    Stat stats_;
};
