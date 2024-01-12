#ifndef RENDERER_UTIL_SMALL_VECTOR_HPP
#define RENDERER_UTIL_SMALL_VECTOR_HPP

namespace renderer {
	template<typename T>
	struct render_vector {
		int Size;
		int Capacity;
		T* Data;

		// Provide standard typedefs but we don't use them ourselves.
		typedef T value_type;
		typedef value_type* iterator;
		typedef const value_type* const_iterator;

		// Constructors, destructor
		render_vector() {
			Size = Capacity = 0;
			Data = nullptr;
		}

		render_vector(const render_vector& src) {
			Size = Capacity = 0;
			Data = nullptr;
			operator=(src);
		}

		render_vector& operator=(const render_vector& src) {
			clear();
			resize(src.Size);
			if (src.Data)
				memcpy(Data, src.Data, (size_t)Size * sizeof(T));
			return *this;
		}

		~render_vector() {
			if (Data)
				free(Data);
		}

		void wipe() {
			if (Data) {
				Size = 0;
				memset(Data, 0, (size_t)Capacity * sizeof(T));
			}
		}

		void clear() {
			if (Data) {
				Size = Capacity = 0;
				free(Data);
				Data = nullptr;
			}
		}

		void clear_delete() {
			for (int n = 0; n < Size; n++)
				free(Data[n]);
			clear();
		}

		void clear_destruct() {
			for (int n = 0; n < Size; n++)
				Data[n].~T();
			clear();
		}

		bool empty() const {
			return Size == 0;
		}

		int size() const {
			return Size;
		}

		int size_in_bytes() const {
			return Size * (int)sizeof(T);
		}

		int max_size() const {
			return 0x7FFFFFFF / (int)sizeof(T);
		}

		int capacity() const {
			return Capacity;
		}

		T& operator[](int i) {
			return Data[i];
		}

		const T& operator[](int i) const {
			return Data[i];
		}

		T* begin() {
			return Data;
		}

		const T* begin() const {
			return Data;
		}

		T* end() {
			return Data + Size;
		}

		const T* end() const {
			return Data + Size;
		}

		T& front() {
			return Data[0];
		}

		const T& front() const {
			return Data[0];
		}

		T& back() {
			return Data[Size - 1];
		}

		const T& back() const {
			return Data[Size - 1];
		}

		void swap(render_vector& rhs) noexcept {
			int rhs_size = rhs.Size;
			rhs.Size = Size;
			Size = rhs_size;
			int rhs_cap = rhs.Capacity;
			rhs.Capacity = Capacity;
			Capacity = rhs_cap;
			T* rhs_data = rhs.Data;
			rhs.Data = Data;
			Data = rhs_data;
		}

		int _grow_capacity(int sz) const {
			int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8;
			return new_capacity > sz ? new_capacity : sz;
		}

		void resize(int new_size) {
			if (new_size > Capacity)
				reserve(_grow_capacity(new_size));

			Size = new_size;
		}

		void resize(int new_size, const T& v) {
			if (new_size > Capacity)
				reserve(_grow_capacity(new_size));

			if (new_size > Size)
				for (int n = Size; n < new_size; n++)
					memcpy(&Data[n], &v, sizeof(v));

			Size = new_size;
		}

		void shrink(int new_size) {
			Size = new_size;
		}

		void reserve(int new_capacity) {
			if (new_capacity <= Capacity)
				return;

			T* new_data = (T*)malloc((size_t)new_capacity * sizeof(T));
			if (Data) {
				memcpy(new_data, Data, (size_t)Size * sizeof(T));
				free(Data);
			}

			Data = new_data;
			Capacity = new_capacity;
		}

		void reserve_discard(int new_capacity) {
			if (new_capacity <= Capacity)
				return;

			if (Data)
				free(Data);

			Data = (T*)malloc((size_t)new_capacity * sizeof(T));
			Capacity = new_capacity;
		}

		// NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the ImVector data
		// itself! e.g. v.push_back(v[10]) is forbidden.
		void push_back(const T& v) {
			if (Size == Capacity)
				reserve(_grow_capacity(Size + 1));

			memcpy(&Data[Size], &v, sizeof(v));
			Size++;
		}

		void pop_back() {
			Size--;
		}

		void push_front(const T& v) {
			if (Size == 0)
				push_back(v);
			else
				insert(Data, v);
		}

		T* erase(const T* it) {
			const ptrdiff_t off = it - Data;
			memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T));
			Size--;
			return Data + off;
		}

		T* erase(const T* it, const T* it_last) {
			const ptrdiff_t count = it_last - it;
			const ptrdiff_t off = it - Data;
			memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - (size_t)count) * sizeof(T));
			Size -= (int)count;
			return Data + off;
		}

		T* erase_unsorted(const T* it) {
			const ptrdiff_t off = it - Data;
			if (it < Data + Size - 1)
				memcpy(Data + off, Data + Size - 1, sizeof(T));

			Size--;
			return Data + off;
		}

		T* insert(const T* it, const T& v) {
			const ptrdiff_t off = it - Data;
			if (Size == Capacity)
				reserve(_grow_capacity(Size + 1));

			if (off < (int)Size)
				memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T));

			memcpy(&Data[off], &v, sizeof(v));
			Size++;
			return Data + off;
		}

		bool contains(const T& v) const {
			const T* data = Data;
			const T* data_end = Data + Size;
			while (data < data_end)
				if (*data++ == v)
					return true;
			return false;
		}

		T* find(const T& v) {
			T* data = Data;
			const T* data_end = Data + Size;
			while (data < data_end) {
				if (*data == v)
					break;

				++data;
			}

			return data;
		}

		const T* find(const T& v) const {
			const T* data = Data;
			const T* data_end = Data + Size;
			while (data < data_end) {
				if (*data == v)
					break;

				++data;
			}

			return data;
		}

		int find_index(const T& v) const {
			const T* data_end = Data + Size;
			const T* it = find(v);
			if (it == data_end)
				return -1;

			const ptrdiff_t off = it - Data;
			return (int)off;
		}

		bool find_erase(const T& v) {
			const T* it = find(v);
			if (it < Data + Size) {
				erase(it);
				return true;
			}

			return false;
		}

		bool find_erase_unsorted(const T& v) {
			const T* it = find(v);
			if (it < Data + Size) {
				erase_unsorted(it);
				return true;
			}

			return false;
		}

		int index_from_ptr(const T* it) const {
			const ptrdiff_t off = it - Data;
			return (int)off;
		}
	};
}// namespace renderer

#endif