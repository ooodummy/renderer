#ifndef _CARBON_LAYOUT_ITEM_HPP_
#define _CARBON_LAYOUT_ITEM_HPP_

#include "axes.hpp"
#include "model.hpp"

#include <glm/vec2.hpp>

// There are so many constructors in here...

namespace carbon {
	enum flex_unit {
		unit_pixel,		// Pixel value
		unit_aspect,	// Aspect to parent main axis size
		unit_relative,	// Aspect relative to another flex_item
		unit_auto		// Ignore value and just clamp basis size to basis content
	};

	// TODO: Maybe add all this fun stuff
	//  I don't really think it has much purpose though
	enum flex_basis_size {
		basis_width,
		basis_percentage,
		basis_auto,
		basis_content,
		basis_fit_content, // (available < max-content) ? max-content : ((available < min-content) ? min-content : available)
		basis_max_content,
		basis_min_content
	};

	enum flex_keyword_values {
		value_initial,
		value_auto,
		value_none
	};

	class flex_item;

	struct flex_width {
		flex_width() = default;
		~flex_width() = default;

		flex_width(float value); // NOLINT(google-explicit-constructor)
		flex_width(flex_unit unit); // NOLINT(google-explicit-constructor)
		flex_width(float value, flex_unit unit);
		flex_width(float value, flex_item* relative);

		flex_unit unit = unit_aspect;
		float value = 0.0f;

		// TODO: How difficult will it actually be to do this?
		flex_item* relative;
	};

	struct flex_basis {
		flex_basis() = default;
		flex_basis(float value);
		flex_basis(flex_unit unit);
		flex_basis(float value, flex_unit unit);
		flex_basis(float value, flex_item* relative);
		flex_basis(bool minimum);

		bool minimum = false; // Same as auto
		glm::vec2 content = {0.0f, 0.0f};
		flex_width width;
	};

	// TODO: Should initial/constructor values be changed?
	struct flex {
		flex() = default;
		flex(float grow);
		flex(float grow, float shrink);
		flex(float grow, float shrink, flex_basis basis);
		flex(flex_basis basis);
		flex(float grow, flex_basis basis);

		// Is this type of code design bad?
		//flex(flex_keyword_values keyword);

		float grow = 0.0f;
		float shrink = 1.0f;
		flex_basis basis;
	};

	// Flexable item with almost all functionality in the standard flex layout model :money_mouth:
	class flex_item : public box_model {
	public:
		flex_item() = default;
		~flex_item() = default;

		friend class base_flex_container;
		friend class flex_line;

		virtual void compute();

		virtual void draw();
		virtual void input();

		virtual void draw_contents();
		virtual void measure_content_min();

		[[nodiscard]] flex_item* get_top_parent() const;
		flex_item* parent = nullptr;

		// Setters and getters are needed so if we can tell if properties have changed since if they have we then need to recompute
		// We can probably remove all the flex constructors since we now have these
		[[nodiscard]] const flex& get_flex() const;
		void set_flex(float grow);
		void set_flex(float grow, float shrink);
		void set_flex(float grow, float shrink, flex_basis basis);
		void set_flex(flex_basis basis);
		void set_flex(float grow, flex_basis basis);

		[[nodiscard]] float get_min_width() const;
		void set_min_width(float min_width);

		[[nodiscard]] float get_max_width() const;
		void set_max_width(float min_width);

		[[nodiscard]] bool get_hidden() const;
		void set_hidden(bool hidden);

		bool disabled = false;

	protected:
		void mark_dirty_and_propagate() override;

		flex flex_;

		float min_width_ = 0.0f;
		float max_width_ = FLT_MAX;

		bool hidden_ = false;

		// Physical size
		glm::vec2 content_min_;

		// flex_line::compute
		float base_size_;
		float hypothetical_size_;
		float shrink_scaled;
		float shrink_ratio;
		float final_size;
		bool flexible;
	};
}

#endif