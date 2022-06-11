#ifndef _CARBON_LAYOUT_ITEM_HPP_
#define _CARBON_LAYOUT_ITEM_HPP_

#include "axes.hpp"
#include "model.hpp"

namespace carbon {
	class flex_item : public box_model {
	public:
		flex_item() = default;
		~flex_item() = default;

		friend class base_flex_container;
		friend class flex_line;

		virtual void compute();

		virtual void draw();
		virtual void input();

		flex_item* get_top_parent() const;
		flex_item* parent = nullptr;

		const flex& get_flex() const;
		void set_flex(const flex& flex);

		float get_min_width() const;
		void set_min_width(float min_width);

		float get_max_width() const;
		void set_max_width(float max_width);

		bool get_hidden() const;
		void set_hidden(bool hidden);

		bool get_disabled() const;
		void set_disabled(bool disabled);

		void mark_dirty_and_propagate() override;

	protected:
		virtual void measure_contents();
		virtual void decorate();

		flex flex_;

		// Should we make min/max a vec2?
		float min_width_ = 0.0f;
		float max_width_ = FLT_MAX;

		// Ignored completely by the layout engine
		bool hidden_ = false;

		// Input disabled
		bool disabled_ = false;

		// Compute data
		glm::vec2 content_min_;		   // Used to provide min content size to parent

		bool flexible_;
		float base_size_; // Starts as the basis size then gets flexed repeatedly if needed
		float final_size_;// Final size, once set we also no longer make the item flexable
		float shrink_scaled_;
		float shrink_ratio_;
	};
}// namespace carbon

#endif