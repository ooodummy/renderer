#ifndef CARBON_WIDGETS_INPUT_LABEL_HPP
#define CARBON_WIDGETS_INPUT_LABEL_HPP

#include "../widget.hpp"

namespace carbon {
	template <typename T>
	class label : public widget_flex_container {
	public:
		label(const T& label, size_t font_id = 0, renderer::color_rgba color = COLOR_WHITE) : font_id_
			(font_id),
			color_
			(color) {
			label_ = label;
			set_flex(0.0f, 0.0f);
			set_basis(flex_unit::unit_pixel);

			container_ = add_child<flex_container>();
			container_->set_flex(0.0f, 0.0f);
			container_->set_basis(flex_unit::unit_pixel);
		}

		void set_label(const T& label) {
			label_ = label;
		}

		void decorate() override {
			auto size = dx11->get_text_size(label_, font_id_);
			size.x += 8.0f;
			size.y += 12.0f;

			if (last_parent_direction != parent->get_flow().main) {
				if (parent->get_flow().main == flex_direction::column) {
					set_min_width(size.y);
					set_basis(size.y);
					set_flow(flex_direction::row);
					container_->set_min_width(size.x);
					container_->set_basis(size.x);
				}
				else {
					set_min_width(size.x);
					set_basis(size.x);
					set_flow(flex_direction::column);
					container_->set_min_width(size.y);
					container_->set_basis(size.y);
				}
				last_parent_direction = parent->get_flow().main;
			}

			auto bounds = container_->get_bounds();
			//buf->draw_rect(get_bounds(), COLOR_GREEN);
			//buf->draw_rect(bounds, COLOR_RED);
			buf->draw_text<T>({bounds.x + bounds.z / 2.0f, bounds.y + bounds.w / 2.0f}, label_, font_id_, color_,
							  renderer::text_align_center, renderer::text_align_center);
		}

	private:
		flex_direction last_parent_direction = flex_direction::undefined;
		flex_container* container_;

		T label_;

		size_t font_id_;
		renderer::color_rgba color_;
	};
}// namespace carbon

#endif