# Build
# mkdir -p build && cd build
# cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
# make -j
#
# Run (two terminals)
# # Terminal 1 (pong): attach to shared memory created by ping
# ./pong /ultra_ipc_demo --cap 1024 --cpu 2
#
# # Terminal 2 (ping): create and benchmark
# ./ping create /ultra_ipc_demo --cap 1024 --cpu 4 --warmup 200000 --msgs 1000000
#
# Expected on a modern x86_64 (isolated cores, no turbo, perf governor performance):
# RTT p50 ~ 300–800 ns, p99 ~ 900–2000 ns
# One-way estimate is roughly half of RTT
#
# Tips for stability
# - Pin ping and pong to separate physical cores (avoid SMT siblings)
# - Use performance governor: `sudo cpupower frequency-set -g performance`
# - Disable Turbo/Boost for determinism; isolate cores via kernel cmdline if possible
# - Close background daemons on a dedicated test box
# - Prefer power-of-two ring sizes that fit comfortably in L1/L2
#
# Extending
# - For MPSC/MCSP, add a cacheline-padded ticketing scheme or use per-producer rings
# - For low-CPU mode, add eventfd for wakeups (adds ~1–3 µs)
# - For NIC ingress, pair with AF_XDP or DPDK and write frames straight into this ring
# - For persistence/replay, append to a memory-mapped log in producer order
