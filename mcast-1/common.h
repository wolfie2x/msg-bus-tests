#pragma once
#include <cstdint>
#include <cstddef>

#define DBG 0   // DEBUG level

#define CMD_NONE    0     // no special command
#define CMD_WARMUP  -1     // warmup message
#define CMD_END     -9     // end of test


// Packed header for each message
struct alignas(8) MsgHeader {
    uint64_t seq;
    uint64_t timestamp_ns;
};

