#ifndef _CARBON_LAYOUT_ITEM_HPP_
#define _CARBON_LAYOUT_ITEM_HPP_

#include "model.hpp"

#include <glm/vec2.hpp>

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

	class flex_item;

	struct flex_value {
		flex_value() = default;
		explicit flex_value(float value);
		explicit flex_value(flex_unit unit);
		flex_value(float value, flex_unit unit);
		flex_value(float value, flex_item* relative);
		~flex_value() = default;

		flex_unit unit;
		float value;

		// TODO: How difficult will it actually be to do this? :/
		flex_item* relative;
	};

	// TODO: Should initial/constructor values be changed?
	struct flex_attributes {
		flex_attributes() = default;
		flex_attributes(float grow); // NOLINT(google-explicit-constructor)
		flex_attributes(float grow, float shrink);
		flex_attributes(float grow, float shrink, flex_value basis);

		flex_attributes(flex_value basis); // NOLINT(google-explicit-constructor)
		flex_attributes(float grow, flex_value basis);

		float grow = 0.0f;
		float shrink = 1.0f;
		bool basis_auto = false;
		glm::vec2 basis_content = { 0.0f, 0.f };
		flex_value basis = flex_value{ unit_aspect };
	};

	// Flexable item with almost all functionality in the standard flex layout model :money_mouth:
	class flex_item : public box_model {
	public:
		flex_item() = default;
		~flex_item() = default;

		friend class flex_line;

		virtual void compute();

		virtual void draw();
		virtual void input();

		virtual void draw_contents();

		[[nodiscard]] flex_item* get_top_parent() const;

		flex_item* parent = nullptr;
		flex_attributes flex;
		bool visible = true; // TODO: Check this when computing and drawing
		float min_width = 0.0f;
		float max_width = FLT_MAX;

	protected:
		// Variables used in flex_line::compute
		float min_content_;
		float base_size_;
		float hypothetical_size_;
		float shrink_scaled;
		float shrink_ratio;
		float final_size;
		bool flexible;
	};
}

#endif