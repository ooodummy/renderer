#ifndef RENDERER_UTIL_UTIL_HPP
#define RENDERER_UTIL_UTIL_HPP

#include <condition_variable>
#include <fmt/core.h>
#include <fmt/xchar.h>
#include <Windows.h>

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
		std::chrono::milliseconds get_elapsed_duration();

	private:
		std::chrono::time_point<std::chrono::steady_clock> begin;
	};

	class performance_counter {
	public:
		performance_counter() {
			QueryPerformanceFrequency(&frequency_);
			QueryPerformanceCounter(&last_time_);

			max_delta_ = frequency_.QuadPart / 10;
		};

		void tick() {
			LARGE_INTEGER current_time;
			QueryPerformanceCounter(&current_time);

			uint64_t delta_time = current_time.QuadPart - last_time_.QuadPart;

			last_time_ = current_time;
			second_counter_ += delta_time;

			if (delta_time > max_delta_)
				delta_time = max_delta_;

			frame_count_++;
			frames_this_second_++;

			if (second_counter_ >= frequency_.QuadPart) {
				frames_per_second_ = frames_this_second_;
				frames_this_second_ = 0;
				second_counter_ %= frequency_.QuadPart;
			}
		}

		uint32_t get_fps() const {
			return frames_per_second_;
		}

	private:
		LARGE_INTEGER frequency_;
		LARGE_INTEGER last_time_;
		uint64_t max_delta_;

		uint32_t frame_count_;
		uint32_t frames_per_second_;
		uint32_t frames_this_second_;
		uint64_t second_counter_;
	};
}// namespace renderer

#endif