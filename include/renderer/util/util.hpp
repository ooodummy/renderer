#ifndef RENDERER_UTIL_UTIL_HPP
#define RENDERER_UTIL_UTIL_HPP

#include <condition_variable>

namespace renderer {
	class sync_manager {
	public:
		void wait() {
			std::unique_lock<std::mutex> lock_guard(mutex);
			condition.wait(lock_guard, [this] { return update; });
			update = false;
		}

		void notify() {
			std::unique_lock<std::mutex> lock_guard(mutex);
			condition.notify_one();
			update = true;
		}

	public:
		std::mutex mutex;
		std::condition_variable condition;
		bool update;
	};

	class timer {
	public:
		timer() {
			reset();
		}

		void reset() {
			begin = std::chrono::high_resolution_clock::now();
		}

		std::chrono::milliseconds get_elapsed_duration() {
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() -
																		 begin);
		}

	private:
		std::chrono::time_point<std::chrono::steady_clock> begin;
	};
}// namespace renderer

#endif