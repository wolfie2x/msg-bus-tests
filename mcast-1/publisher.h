
 #pragma once
#include "common.h"
#include "ITransport.h"
#include <netinet/in.h>

class Publisher {
public:
    Publisher(const char* mcast_addr, uint16_t port);
    ~Publisher();
    // Send payload, only length passed to transport
    bool send(size_t len, int msg_cmd = CMD_NONE);
private:
    ITransport* transport_ = nullptr;
    uint64_t app_seq_ = 0;
};
