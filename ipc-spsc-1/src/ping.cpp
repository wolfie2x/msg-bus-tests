#include <iostream>
auto run = [&](auto cap_tag){
constexpr uint64_t CAP = decltype(cap_tag)::value;
using Region = SharedRegion<CAP>;
bool create = (mode == "create");
Shm shm{name, sizeof(Region), create};
auto* region = reinterpret_cast<Region*>(shm.addr());


// Warmup (fill/echo to steady state)
Msg m{}; m.seq = 0; m.t0_ns = 0;
for(uint64_t i=0;i<warmup;i++){
m.seq = i; m.t0_ns = now_ns();
region->req.push_spin(m);
region->rsp.pop_spin(m);
}


Stats stats; stats.samples.reserve(msgs);
for(uint64_t i=0;i<msgs;i++){
m.seq = i; m.t0_ns = now_ns();
region->req.push_spin(m);
region->rsp.pop_spin(m);
uint64_t t1 = now_ns();
uint64_t rtt = t1 - m.t0_ns;
record(stats, rtt);
}


// Percentiles
auto p50 = percentile(stats.samples, 0.50);
auto p90 = percentile(stats.samples, 0.90);
auto p99 = percentile(stats.samples, 0.99);
auto p999 = percentile(stats.samples, 0.999);


std::cout << "Messages: " << stats.count << "\n";
std::cout << "RTT ns min=" << stats.min_ns << " p50=" << p50
<< " p90=" << p90 << " p99=" << p99 << " p99.9=" << p999
<< " max=" << stats.max_ns << "\n";
std::cout << "One-way estimate (ns): p50=" << p50/2 << " p99=" << p99/2 << "\n";
};


struct C1{ static constexpr uint64_t value=1024;};
struct C2{ static constexpr uint64_t value=2048;};
struct C4{ static constexpr uint64_t value=4096;};
if (cap <= 1024) run(C1{});
else if (cap <= 2048) run(C2{});
else run(C4{});
}