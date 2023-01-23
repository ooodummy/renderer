#ifndef CARBON_LAYOUT_ITEM_HPP
#define CARBON_LAYOUT_ITEM_HPP

#include "axes.hpp"
#include "model.hpp"

namespace carbon {
	class flex_item : public box_model {
	public:
		flex_item() = default;
		~flex_item() = default;

		friend class base_flex_container;
		friend class flex_container;

		virtual void compute();

		virtual void draw();
		virtual void input();

		base_flex_container* get_top_parent() const;
		base_flex_container* parent = nullptr;

		const flex& get_flex() const;

		void set_flex(float grow);
		void set_flex(float grow, float shrink);
		void set_flex(float grow, float shrink, flex_basis basis);
		void set_flex(flex_basis basis);
		void set_flex(float grow, flex_basis basis);

		void set_basis(float value);
		void set_basis(flex_unit unit);
		void set_basis(float value, flex_unit unit);
		void set_basis(bool minimum);

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
		glm::vec2 content_min_;// Used to provide min content size to parent

		bool frozen_;
		float base_size_;
		float hypothetical_size_;
		float target_size_;
		float scaled_shrink_factor_;
		float scaled_shrink_factor_ratio_;
		float adjustment_;
	};
}// namespace carbon

#endif