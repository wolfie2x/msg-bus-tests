#pragma once
#include "common.h"
#include "stat.h"
#include "ITransport.h"


class Subscriber : public ITransportCB {
public:

    Subscriber(Stat* stats, ITransport* transport);
    ~Subscriber(){};
    
    // ITransportCB implementation
    void on_data(const void* data, size_t len, uint64_t mc_seq) override;

private:
    Stat* stats_;
    ITransport* transport_ = nullptr;
};
