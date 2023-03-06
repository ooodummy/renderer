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
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin);
}

renderer::performance_counter::performance_counter() {
	QueryPerformanceFrequency(&frequency_);
	QueryPerformanceCounter(&last_time_);
}

void renderer::performance_counter::tick() {
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);

	delta_time_ = static_cast<uint64_t>(current_time.QuadPart - last_time_.QuadPart);
	delta_time_ *= ticks_per_second_;
	delta_time_ /= static_cast<uint64_t>(frequency_.QuadPart);

	second_counter_ += current_time.QuadPart - last_time_.QuadPart;
	last_time_ = current_time;

	frame_count_++;
	frames_this_second_++;

	if (second_counter_ >= frequency_.QuadPart) {
		frames_per_second_ = frames_this_second_;
		frames_this_second_ = 0;
		second_counter_ %= frequency_.QuadPart;
	}
}

uint32_t renderer::performance_counter::get_fps() const {
	return frames_per_second_;
}

float renderer::performance_counter::get_dt() const {
	return static_cast<float>(delta_time_) / static_cast<float>(ticks_per_second_);
}
