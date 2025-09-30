#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <cstddef>
#include <cstdint>


enum class RecvMode {
    Blocking,
    Polling
};

class ITransportCB {
public:
    virtual ~ITransportCB() = default;
    virtual void on_data(const void* data, size_t len, uint64_t mc_seq) = 0;
};

class ITransport {
public:
    virtual ~ITransport(){};
    virtual void* get_send_buffer(size_t len) = 0;
    // Send data. Returns true on success.
    virtual bool send(size_t len) = 0;
    // Receive data. Returns number of bytes received, or -1 on error.
    virtual int run_recv_loop(RecvMode mode = RecvMode::Blocking) = 0;
    // Register callback for data arrival
    virtual void set_callback(ITransportCB* cb) = 0;
};

#endif // ITRANSPORT_H
