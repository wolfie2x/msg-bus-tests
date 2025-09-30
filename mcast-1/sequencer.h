#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "ITransport.h"
#include "common.h"

/**
 * Sequencer component that receives messages on one port and forwards them on another port.
 * 
 * The Sequencer:
 * - Listens for incoming messages on a multicast address and recv_port
 * - Timestamps each message in ts1_ns field before forwarding  
 * - Stamps each message with a sequencer sequence number in tx_id field
 * - Forwards messages to the same multicast address but on send_port
 * - Uses polling mode for lowest latency
 * - Does not collect statistics (unlike Publisher/Subscriber)
 * 
 * Usage: ./sequencer <recv_mcast_addr> <recv_port> <send_port>
 */
class Sequencer : public ITransportCB {
public:
    explicit Sequencer(ITransport* transport);
    ~Sequencer() = default;

    // ITransportCB interface
    void on_data(int64_t sender_id, const void* data, size_t len, int64_t mc_seq) override;

private:
    ITransport* transport_;
    int64_t tx_seq_ = 0;  // sequencer sequence number
};

#endif // SEQUENCER_H