#ifndef OKP_RECOGNITION_TIMER_H
#define OKP_RECOGNITION_TIMER_H

#include <atomic>
#include <chrono>

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline size_t to_ns(const D &d) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
}

#endif //OKP_RECOGNITION_TIMER_H
