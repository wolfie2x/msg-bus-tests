#pragma once
#include <vector>
#include <inttypes.h>
#include <cstdint>
#include <string>
#include <fstream>
#include <cassert>

class Stat {
public:
    Stat(size_t max_msgs) : records_(max_msgs), idx_(0) {}

    // Record sequence number and latency in nanoseconds
    void record(uint64_t seq, int64_t latency_ns) {
        if (idx_ >= records_.size()) {
            size_t new_size = records_.size() * 2;
            fprintf(stderr, "Warning: Stat::record exceeded preallocated size (%zu), resizing to %zu (seq=%lu)\n", records_.size(), new_size, (unsigned long)seq);
            records_.resize(new_size);
        }
        records_[idx_++] = {seq, latency_ns};
    }

    // Write all recorded seq/latency pairs to a CSV file
    void write_csv(const std::string& filename) const {
        std::ofstream out(filename);
        out << "seq,latency_ns\n";
        for (size_t i = 0; i < idx_; ++i) {
            out << records_[i].seq << "," << records_[i].latency << "\n";
        }
        out.close();
    }

    size_t size() const { return idx_; }

private:
    struct Record {
        uint64_t seq;
        int64_t latency;
    };
    std::vector<Record> records_;
    size_t idx_;
};
