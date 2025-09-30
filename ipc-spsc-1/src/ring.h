#ifndef ULTRAIPC_RING_HPP


namespace ultraipc {


// Cacheline size assumption
static constexpr size_t CACHELINE = 64;


template <typename T, uint64_t CapacityPowerOfTwo>
struct alignas(CACHELINE) SpscRing {
static_assert((CapacityPowerOfTwo & (CapacityPowerOfTwo - 1)) == 0, "Capacity must be power-of-two");
static constexpr uint64_t CAP = CapacityPowerOfTwo;
static constexpr uint64_t MASK = CAP - 1;


struct alignas(CACHELINE) Index { std::atomic<uint64_t> value; char pad[CACHELINE - sizeof(std::atomic<uint64_t>)]; };


Index head; // next slot to write by producer
Index tail; // next slot to read by consumer


// Slots (POD recommended). For non-POD, placement new/dtor would be needed.
T slots[CAP];


SpscRing() { head.value.store(0, std::memory_order_relaxed); tail.value.store(0, std::memory_order_relaxed); }


bool try_push(const T& item) {
uint64_t h = head.value.load(std::memory_order_relaxed);
uint64_t t = tail.value.load(std::memory_order_acquire);
if ((h - t) >= CAP) return false; // full
slots[h & MASK] = item; // copy payload
head.value.store(h + 1, std::memory_order_release);
return true;
}


bool try_pop(T& out) {
uint64_t t = tail.value.load(std::memory_order_relaxed);
uint64_t h = head.value.load(std::memory_order_acquire);
if (t == h) return false; // empty
out = slots[t & MASK];
tail.value.store(t + 1, std::memory_order_release);
return true;
}


// Busy-wait helpers for lowest latency
void push_spin(const T& item) {
while (!try_push(item)) { ultraipc::busy_pause(); }
}
void pop_spin(T& out) {
while (!try_pop(out)) { ultraipc::busy_pause(); }
}
};


} // namespace ultraipc


#endif // ULTRAIPC_RING_HPP
