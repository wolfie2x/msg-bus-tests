#pragma once
#include "common.h"
#include "stat.h"
#include "ITransport.h"


class Subscriber : public ITransportCB {
public:

    Subscriber(Stat* stats, ITransport* transport);
    ~Subscriber(){};
    
    void check_end();

    // ITransportCB implementation
    void on_data(int64_t sender_id, const void* data, size_t len, int64_t mc_seq) override;

private:
    Stat* stats_;
    ITransport* transport_ = nullptr;

    int64_t senders[MAX_SENDERS] = {0};  // track active senders
};
