#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <chrono>
#include <string>
#include <iostream>

// Phase 2 optimization: Performance benchmarking framework
class PerformanceBenchmark {
    std::chrono::high_resolution_clock::time_point start_;
    std::string operation_name_;

public:
    explicit PerformanceBenchmark(std::string name)
        : start_(std::chrono::high_resolution_clock::now())
        , operation_name_(std::move(name)) {
    }

    ~PerformanceBenchmark() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        std::cout << "[BENCHMARK] " << operation_name_ << " took: " << duration.count() << " μs\n";
    }

    // Manual timing control
    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }

    [[nodiscard]] long long elapsed_microseconds() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
    }
};

#define BENCHMARK(name) PerformanceBenchmark bench(name)
#define BENCHMARK_BLOCK(name, block) { PerformanceBenchmark bench(name); block; }

// Memory usage tracking
class MemoryProfiler {
    static size_t peak_memory_usage_;
    static size_t current_memory_usage_;

public:
    static void recordAllocation(size_t size) {
        current_memory_usage_ += size;
        peak_memory_usage_ = std::max(peak_memory_usage_, current_memory_usage_);
    }

    static void recordDeallocation(size_t size) {
        current_memory_usage_ -= size;
    }

    static size_t getPeakUsage() { return peak_memory_usage_; }
    static size_t getCurrentUsage() { return current_memory_usage_; }
    static void reset() { peak_memory_usage_ = current_memory_usage_ = 0; }
};

#endif // BENCHMARK_HPP
