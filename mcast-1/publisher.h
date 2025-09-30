
 #pragma once
#include "common.h"
#include "ITransport.h"
#include <netinet/in.h>

class Publisher {
public:
    Publisher(ITransport* transport);    
    ~Publisher();

    void Init(int msg_size, int msg_rate, int msg_count);
    void start_publishing();

    // Send payload, only length passed to transport
    bool send(int64_t ts_ns, int64_t msg_cmd = CMD_NONE);
private:
    ITransport* transport_ = nullptr;
    uint64_t app_seq_ = 0;
    int msg_size_ = 0;
    int msg_rate_ = 0;
    int msg_count_ = 0;
};
