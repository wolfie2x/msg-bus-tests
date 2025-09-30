#include <iostream>


using namespace ultraipc;


static void usage(const char* prog){
std::cerr << "Usage: " << prog << " <shm_name> [--cap 1024] [--cpu -1]\n";
}


int main(int argc, char** argv){
if (argc < 2) { usage(argv[0]); return 1; }
std::string name = argv[1];
uint64_t cap = 1024;
int cpu = -1;
for (int i=2;i<argc;i++){
std::string a = argv[i];
if (a == "--cap" && i+1 < argc) { cap = std::stoull(argv[++i]); }
else if (a == "--cpu" && i+1 < argc) { cpu = std::stoi(argv[++i]); }
}


// Round capacity up to power-of-two at compile time via template instantiation switch
// Keep a few common sizes to avoid templates explosion at runtime
pin_to_cpu(cpu);


// Helper lambda to run for a given capacity via template
auto run = [&](auto cap_tag){
constexpr uint64_t CAP = decltype(cap_tag)::value;
using Region = SharedRegion<CAP>;
Shm shm{name, sizeof(Region), false};
auto* region = reinterpret_cast<Region*>(shm.addr());
Msg m;
for(;;){
region->req.pop_spin(m); // read request
// Echo back with same seq/timestamp, minimal work
region->rsp.push_spin(m);
}
};


struct C1{ static constexpr uint64_t value=1024;};
struct C2{ static constexpr uint64_t value=2048;};
struct C4{ static constexpr uint64_t value=4096;};
if (cap <= 1024) run(C1{});
else if (cap <= 2048) run(C2{});
else run(C4{});
}