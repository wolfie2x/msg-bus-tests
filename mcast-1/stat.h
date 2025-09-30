#pragma once
#include <vector>
#include <inttypes.h>
#include <cstdint>
#include <string>
#include <fstream>
#include <cassert>
#include <cstdio>

class Stat {
public:
    Stat(size_t max_msgs, const std::string& filename) : 
        records_(max_msgs), idx_(0), filename_(filename) {}

    // Record sequence number and latency in nanoseconds
    void record(int64_t sender, int64_t seq, int64_t latency_ns) {
        if (idx_ >= records_.size()) {
            size_t new_size = records_.size() * 2;
            fprintf(stderr, "Warning: Stat::record exceeded preallocated size (%zu), resizing to %zu (seq=%ld)\n", records_.size(), new_size, (long)seq);
            records_.resize(new_size);
        }
        records_[idx_++] = {sender, seq, latency_ns};
    }

    // Write all recorded seq/latency pairs to a CSV file
    void write_csv() const {
        std::ofstream out(filename_);
        out << "sender,seq,latency_ns\n";
        for (size_t i = 0; i < idx_; ++i) {
            out << records_[i].sender << "," << records_[i].seq << "," << records_[i].latency << "\n";
        }
        out.close();
        printf("Stats: Latencies written to file %s\n", filename_.c_str());
    }

    size_t size() const { return idx_; }

private:
    struct Record {
        int64_t sender;
        int64_t seq;
        int64_t latency;
    };
    std::vector<Record> records_;
    size_t idx_;
    std::string filename_;
};
