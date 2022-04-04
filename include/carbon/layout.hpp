#ifndef _CARBON_LAYOUT_HPP_
#define _CARBON_LAYOUT_HPP_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <vector>

// Concepts found at
// https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout/Basic_Concepts_of_Flexbox

namespace carbon {
	enum flex_axis {
		flex_axis_row,
		flex_axis_column
	};

	enum flex_direction {
		flex_direction_forward,
		flex_direction_reversed
	};

	enum flex_align {
		flex_align_stretch,
		flex_align_start,
		flex_align_end,
		flex_align_center
	};

	enum flex_justify_content {
		flex_justify_start,
		flex_justify_end,
		flex_justify_center,
		flex_justify_space_around,
		flex_justify_space_between,
		flex_justify_space_evenly
	};

	// TODO: Handle auto
	enum flex_grid_resize {
		flex_grid_resize_none,
		flex_grid_resize_row,
		flex_grid_resize_column,
		flex_grid_resize_auto
	};

	enum flex_wrap {
		flex_nowrap,
		flex_warp
	};

	class flex_item {
	public:
		flex_item() = default;
		~flex_item() = default;

		virtual void compute();

		// Used for widgets
		virtual void draw();
		virtual void input();

		void draw_contents();

		void set_parent(flex_item* item);
		[[nodiscard]] flex_item* get_parent() const;
		[[nodiscard]] flex_item* get_top_parent() const;

		template<typename T, typename... Args>
		T* add_child(Args&&... args) {
			return reinterpret_cast<T*>(add_child(std::unique_ptr<T>(new T(std::forward<Args>(args)...))));
		}

		carbon::flex_item* add_child(std::unique_ptr<flex_item> item);
		[[nodiscard]] std::vector<std::unique_ptr<flex_item>>& get_children();

		void set_pos(glm::vec2 pos);
		[[nodiscard]] glm::vec2 get_pos() const;

		void set_size(glm::vec2 size);
		[[nodiscard]] glm::vec2 get_size() const;

		[[nodiscard]] glm::vec4 get_bounds() const;

		void set_margin(glm::vec2 margin);
		void set_margin(float margin);
		[[nodiscard]] glm::vec2 get_margin() const;

		void set_padding(glm::vec2 padding);
		void set_padding(float padding);
		[[nodiscard]] glm::vec2 get_padding() const;

		// Used by flex containers
		void set_min(glm::vec2 min);
		[[nodiscard]] glm::vec2 get_min() const;

		void set_max(glm::vec2 max);
		[[nodiscard]] glm::vec2 get_max() const;

		void set_grow(float grow);
		[[nodiscard]] float get_grow() const;

		void set_shrink(float shrink);
		[[nodiscard]] float get_shrink() const;

		glm::vec2 override_size_axes = { 0.0f, 0.0f };

	protected:
		flex_item* parent_;
		std::vector<std::unique_ptr<flex_item>> children_;

		glm::vec2 pos_{};
		glm::vec2 size_{};

		glm::vec2 padding_ = { 0.0f, 0.0f };
		glm::vec2 margin_ = { 0.0f, 0.0f };

		// x will always be horizontal and y vertical
		// we will not use the main or cross axes to make
		// these calculations complex
		glm::vec2 min_ = { 0.0f, 0.0f };
		glm::vec2 max_ = { FLT_MAX, FLT_MAX };

		// TODO: Understand flex basis better and impl shrink
		float grow_ = 0.0f;

		float shrink_ = 0.0f;
	};

	class base_container : public flex_item {
	protected:
		static float get_sum(glm::vec2 src);
		static glm::vec2 get_axes_sum(glm::vec4 src);

		static float get_axis(flex_axis axis, glm::vec2 src);
		static glm::vec2 get_axis(flex_axis axis, glm::vec4 src);
		static void set_axis(flex_axis axis, glm::vec2& dst, float src);
		static void set_axis(flex_axis axis, glm::vec4& dst, glm::vec2 src);

		[[nodiscard]] float get_main(glm::vec2 src) const;
		[[nodiscard]] glm::vec2 get_main(glm::vec4 src) const;
		void set_main(glm::vec2& dst, float src);
		void set_main(glm::vec4& dst, glm::vec2 src);

		[[nodiscard]] float get_cross(glm::vec2 src) const;
		[[nodiscard]] glm::vec2 get_cross(glm::vec4 src) const;
		void set_cross(glm::vec2& dst, float src);
		void set_cross(glm::vec4& dst, glm::vec2 src);

		[[nodiscard]] glm::vec2 get_axes(glm::vec2 src) const;
		[[nodiscard]] glm::vec4 get_axes(glm::vec4 src) const;
		void set_axes(glm::vec2& dst, glm::vec2 src);
		void set_axes(glm::vec4& dst, glm::vec4 src);

		flex_axis main_axis_ = flex_axis_row;
		flex_axis cross_axis_ = flex_axis_column;
	};

	class grid_container : public base_container {
	public:
		void compute() override;

		void set_grid_size(glm::i16vec2 grid);

		void set_row_direction(flex_direction direction);
		void set_column_direction(flex_direction direction);

		void set_resize(flex_grid_resize resize);

	protected:
		glm::i16vec2 get_grid_start();

		glm::i16vec2 grid_size_ = { 0, 0 };

		flex_direction row_direction_ = flex_direction_forward;
		flex_direction column_direction_ = flex_direction_forward;

		flex_grid_resize resize_ = flex_grid_resize_none;

		// TODO: Row scales and mins
		//  Functions to get cell at pos
		std::vector<glm::vec4> cell_bounds_list_;
	};

	class flex_container : public base_container {
	public:
		void compute() override;

		void set_axis(flex_axis axis);
		void set_direction(flex_direction direction);
		void set_align(flex_align align);
		void set_justify_context(flex_justify_content justify_content);

	private:
		flex_direction direction_ = flex_direction_forward;

		flex_align align_ = flex_align_start;
		flex_justify_content justify_content_ = flex_justify_start;
	};
}// namespace carbon

#endif