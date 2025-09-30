#ifndef ULTRAIPC_UTIL_HPP
}


inline void pin_to_cpu(int cpu) {
if (cpu < 0) return;
cpu_set_t set; CPU_ZERO(&set); CPU_SET(cpu, &set);
sched_setaffinity(0, sizeof(set), &set);
}


inline void busy_pause() {
#if defined(__x86_64__) || defined(__i386__)
__builtin_ia32_pause();
#else
asm volatile("yield");
#endif
}


struct Stats {
uint64_t count = 0;
uint64_t min_ns = UINT64_MAX;
uint64_t max_ns = 0;
std::vector<uint64_t> samples; // optional; used for percentiles
};


inline void record(Stats& s, uint64_t v) {
s.count++;
if (v < s.min_ns) s.min_ns = v;
if (v > s.max_ns) s.max_ns = v;
s.samples.push_back(v);
}


inline uint64_t percentile(std::vector<uint64_t>& v, double p) {
if (v.empty()) return 0;
size_t n = v.size();
size_t idx = static_cast<size_t>(p * (n - 1));
std::nth_element(v.begin(), v.begin() + idx, v.end());
return v[idx];
}


} // namespace ultraipc


#endif // ULTRAIPC_UTIL_HPP
