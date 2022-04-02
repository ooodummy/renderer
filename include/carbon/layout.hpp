#ifndef _CARBON_LAYOUT_HPP_
#define _CARBON_LAYOUT_HPP_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <vector>

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

	enum flex_grid_resize {
		flex_grid_resize_none,
		flex_grid_resize_row,
		flex_grid_resize_column
	};

	enum flex_wrap {
		flex_nowrap,
		flex_warp
	};

	// TODO:
	//  Wrapping will not be handled but shouldn't be needed in the UI except for text.
	//  When labels are drawn with renderer they will have a layout_item that has a impl for getting the glyph bounds
	//  and will automatically add newlines.
	class layout_item : public std::enable_shared_from_this<layout_item> {
	public:
		layout_item() = default;
		~layout_item() = default;

		virtual void compute();

		void set_parent(std::shared_ptr<layout_item> item);
		std::shared_ptr<layout_item> get_parent() const;
		std::shared_ptr<layout_item> get_top_parent() const;

		void set_pos(glm::vec2 pos);
		glm::vec2 get_pos() const;

		void set_size(glm::vec2 size);
		glm::vec2 get_size() const;

		glm::vec4 get_bounds() const;

		void set_margin(glm::vec4 margin);
		void set_margin(glm::vec2 margin);
		void set_margin(float margin);
		glm::vec4 get_margin() const;

		void set_padding(glm::vec4 padding);
		void set_padding(glm::vec2 padding);
		void set_padding(float padding);
		glm::vec4 get_padding() const;

	protected:
		std::shared_ptr<layout_item> parent_;

		glm::vec2 pos_{};
		glm::vec2 size_{};

		// Spacing format will be like this
		// should be self-explanatory without an example
		//      y
		//   +-----+
		// x |     | z
		//   +-----+
		//      w
		glm::vec4 padding_ = { 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec4 margin_ = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	class grid_container;
	class flex_container;

	// All derived one's need flex layout data since a container can be a flexbox item
	class flex_item : public layout_item {
	public:
		std::shared_ptr<flex_item> add_child(std::shared_ptr<flex_item> item);
		std::vector<std::shared_ptr<flex_item>> get_children() const;

		std::shared_ptr<flex_item> add_flex_item();
		std::shared_ptr<grid_container> add_grid_container();
		std::shared_ptr<flex_container> add_flex_container();

		void set_min(glm::vec2 min);
		glm::vec2 get_min() const;

		void set_max(glm::vec2 max);
		glm::vec2 get_max() const;

		void set_grow(float grow);
		float get_grow() const;

		void set_shrink(glm::vec2 shrink);
		glm::vec2 get_shrink() const;

	protected:
		std::vector<std::shared_ptr<flex_item>> children_;

		glm::vec2 min_ = { 0.0f, 0.0f };
		glm::vec2 max_ = { FLT_MAX, FLT_MAX };

		// https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout/Basic_Concepts_of_Flexbox#the_flex-grow_property
		float grow_ = 0.0f;

		// https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout/Basic_Concepts_of_Flexbox#the_flex-shrink_property
		glm::vec2 shrink_ = { 0.0f, 0.0f };
	};

	class grid_container : public flex_item {
	public:
		void compute() override;

		// TODO: Options to set specific row sizes
		void set_grid(glm::i16vec2 grid);

		void set_row_direction(flex_direction direction);
		void set_column_direction(flex_direction direction);

		void set_resize(flex_grid_resize resize);

	private:
		glm::i16vec2 get_grid_start();
		void increment_and_resize_grid(glm::i16vec2& pos);

		glm::i16vec2 grid_ = { 0, 0 };

		flex_direction row_direction_ = flex_direction_forward;
		flex_direction column_direction_ = flex_direction_forward;

		flex_grid_resize resize_ = flex_grid_resize_none;
	};

	class flex_container : public flex_item {
	public:
		void compute() override;

		void set_axis(flex_axis axis);
		void set_direction(flex_direction direction);
		void set_align(flex_align align);
		void set_justify_context(flex_justify_content justify_content);

	private:
		static float get_sum(glm::vec2 src);
		static glm::vec2 get_sum(glm::vec4 src);

		static float get_axis(flex_axis axis, glm::vec2 src);
		static glm::vec2 get_axis(flex_axis axis, glm::vec4 src);

		static void set_axis(flex_axis axis, glm::vec2& dst, float src);
		static void set_axis(flex_axis axis, glm::vec4& dst, glm::vec2 src);

		float get_main(glm::vec2 src) const;
		glm::vec2 get_main(glm::vec4 src) const;
		void set_main(glm::vec2& dst, float src) const;
		void set_main(glm::vec4& dst, glm::vec2 src) const;

		float get_cross(glm::vec2 src) const;
		glm::vec2 get_cross(glm::vec4 src) const;
		void set_cross(glm::vec2& dst, float src) const;
		void set_cross(glm::vec4& dst, glm::vec2 src) const;

		glm::vec2 get_axes(glm::vec2 src) const;
		glm::vec4 get_axes(glm::vec4 src) const;
		void set_axes(glm::vec2& dst, glm::vec2 src) const;
		void set_axes(glm::vec4& dst, glm::vec4 src) const;

		flex_axis main_axis_ = flex_axis_row;
		flex_axis cross_axis_ = flex_axis_column;

		flex_direction direction_ = flex_direction_forward;

		flex_align align_ = flex_align_start;
		flex_justify_content justify_content_ = flex_justify_start;
	};
}// namespace carbon

#endif