#ifndef _RENDERER_UTIL_SMALL_VECTOR_HPP_
#define _RENDERER_UTIL_SMALL_VECTOR_HPP_

namespace renderer {
	// Probably bad code, but it works.
	// Stopped working on this currently.
	template <typename T, size_t StartSize, size_t IncrementSize>
	class small_vector {
	public:
		small_vector() {
			buf = new T[capacity_];
		}

		~small_vector() {
			delete buf;
		}

		void resize(size_t new_size) {
			if (size > capacity_) {
				capacity_ = size + IncrementSize;
				T* new_buf = new T[capacity_];
				for (size_t i = 0; i < size; i++) {
					new_buf[i] = buf[i];
				}
				delete buf;
				buf = new_buf;
			}
		}

		void push_bacK(const T& val) {
			if (size == capacity_) {
				resize(capacity_ + IncrementSize);
			}

			buf[size++] = val;
		}

		T* buf;
		size_t size = 0;

	private:
		size_t capacity_ = StartSize;
	};
}

#endif