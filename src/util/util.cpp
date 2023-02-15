#include "renderer/util/util.hpp"

void renderer::sync_manager::wait() {
	std::unique_lock<std::mutex> lock_guard(mutex_);
	condition_.wait(lock_guard, [this] { return update_; });
	update_ = false;
}

void renderer::sync_manager::notify() {
	std::unique_lock<std::mutex> lock_guard(mutex_);
	condition_.notify_one();
	update_ = true;
}

renderer::timer::timer() {
	reset();
}

void renderer::timer::reset() {
	begin = std::chrono::high_resolution_clock::now();
}

std::chrono::milliseconds renderer::timer::get_elapsed_duration() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() -
																 begin);
}
