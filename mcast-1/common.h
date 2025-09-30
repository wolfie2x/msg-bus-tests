#pragma once
#include <cstdint>
#include <cstddef>

// Debug level
#define DBG             0   // 0: none, 2: msg level debug info

#define SEQ_ZERO_COPY   1    // 1: enable zero-copy in sequencer, 0: use memcpy



#define MAX_SENDERS     8   // max number of senders supported


// Thread core assignments
#define SUB_CORE  1  // subscriber thread core
#define PUB_CORE  2  // publisher thread core
#define SEQ_CORE  3  // sequencer thread core


// all receivers will reset last received seq to 0 on seeing this value
// #define SEQ_RESET    -999

// Message commands
#define CMD_NONE    0     // no special command
#define CMD_WARMUP  -1     // warmup message
#define CMD_END     -9     // end of test


// App msg content
struct alignas(8) AppMsgData {
    int64_t app_seq;
    int64_t msg_cmd;
    int64_t ts0_ns;  // send timestamp in ns
    int64_t ts1_ns;  // echo timestamp in ns
    int64_t tx_id;   // sequence no. stamped by the sequencer.
};

