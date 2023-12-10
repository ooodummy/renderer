#ifndef RENDERER_UTIL_UTIL_HPP
#define RENDERER_UTIL_UTIL_HPP

#include <Windows.h>
#include <condition_variable>
#include <fmt/core.h>
#include <fmt/xchar.h>

#ifdef _DEBUG
#define DPRINTF(text, ...) fmt::print(text, __VA_ARGS__)
#else
#define DPRINTF(text, ...)
#endif

namespace renderer {
    class sync_manager {
    public:
        void wait();

        void notify();

    private:
        std::mutex mutex_;
        std::condition_variable condition_;
        bool update_;
    };

    class timer {
    public:
        timer();

        void reset();

        template <typename Duration = std::chrono::milliseconds>
        Duration get_elapsed_duration() const {
            return std::chrono::duration_cast<Duration>(std::chrono::high_resolution_clock::now() - begin);
        }

    private:
        std::chrono::time_point<std::chrono::steady_clock> begin;
    };

    class performance_counter {
    public:
        performance_counter();

        void tick();

        uint32_t get_fps() const;

        float get_dt() const;

    private:
        static constexpr uint64_t ticks_per_second_ = 10000000;

        LARGE_INTEGER frequency_{};
        LARGE_INTEGER last_time_{};

        uint64_t delta_time_;

        uint32_t frame_count_;
        uint32_t frames_per_second_;
        uint32_t frames_this_second_;
        uint64_t second_counter_;
    };
}// namespace renderer

#endif