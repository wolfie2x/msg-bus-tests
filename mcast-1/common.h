#pragma once
#include <cstdint>
#include <cstddef>

#define DBG 0   // DEBUG level

#define SUB_CORE  1  // subscriber thread core
#define PUB_CORE  2  // publisher thread core

#define CMD_NONE    0     // no special command
#define CMD_WARMUP  -1     // warmup message
#define CMD_END     -9     // end of test


// App msg content
struct alignas(8) AppMsgData {
    int64_t app_seq;
    int64_t msg_cmd;
    int64_t ts0_ns;  // send timestamp in ns
    int64_t ts1_ns;  // echo timestamp in ns
};

