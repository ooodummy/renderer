#ifndef RENDERER_UTIL_UTIL_HPP
#define RENDERER_UTIL_UTIL_HPP

#include <condition_variable>

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
}// namespace renderer

#endif