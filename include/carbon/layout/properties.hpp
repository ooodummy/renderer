#ifndef CARBON_LAYOUT_PROPERTIES_HPP
#define CARBON_LAYOUT_PROPERTIES_HPP

#include <glm/glm.hpp>

// https://www.w3.org/TR/css-flexbox-1
// All the constructors for flex properties try to imitate the CSS flex shorthand due to the need to mark items as dirty
// when modified I don't think using set values with the whole property would be ideal because singular variables
// couldn't be set one after another. So I think I will resort to creating an eye watering amount of constructors.
// Mercury

namespace carbon {
	enum flex_unit {
		unit_pixel,	  // Pixel value
		unit_aspect,  // Aspect to parent main axis size
		unit_relative,// Aspect relative to another flex_item
		unit_auto	  // Ignore value and just clamp basis size to basis content
	};

	// TODO: Would doing this be appropriate?
	enum flex_basis_size {
		basis_width,        // Use width as basis
		basis_percentage,   // Use percentage of parent main axis size as basis
		basis_auto,         // Use min size as basis
		basis_content,      // Use content size as basis
		basis_fit_content,  // (available < max-content) ? max-content : ((available < min-content) ? min-content :
							// available)
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

		flex_width(float value);
		flex_width(flex_unit unit);
		flex_width(float value, flex_unit unit);
		// flex_width(float value, flex_item* relative);

		flex_unit unit = unit_aspect;
		float value = 0.0f;

		// Will implementing relative widths be worth it?
		// And what are the guidelines for computing them?
		// flex_item* relative;
	};

	struct flex_basis {
		flex_basis() = default;
		flex_basis(float value);
		flex_basis(flex_unit unit);
		flex_basis(float value, flex_unit unit);
		flex_basis(bool minimum);

		bool minimum = false;
		glm::vec2 content = { 0.0f, 0.0f };
		flex_width width;
	};

	class flex_item;

	struct flex {
		flex() = default;
		~flex() = default;

		flex(float grow);
		flex(float grow, float shrink);
		flex(float grow, float shrink, flex_basis basis);
		flex(flex_basis basis);
		flex(float grow, flex_basis basis);
		flex(flex_keyword_values keyword);

		float grow = 0.0f;
		float shrink = 1.0f;
		flex_basis basis;
	};

	enum flex_wrap {
		no_wrap,        // Break flex items out of line if it cannot shrink to fit the space
		wrap,           // Wrap to new line when exceeds content main
		wrap_reverse
	};

	// Alignment on the cross axis
	enum flex_align {
		align_start,    // Align to start of cross axis
		align_end,      // Align to end of cross axis
		align_center,   // Align to center of cross axis
		align_stretch,  // Fill cross axis
		align_baseline  // Align to baseline of items inside
	};

	// Alignment on the main axis
	enum flex_justify_content {
		justify_start,
		justify_end,
		justify_center,
		justify_space_around, // Items have a half-size space on either end
		justify_space_between,// The first item is flush with the start, the last is flush with the end
		justify_space_evenly, // Items have equal space around them
		justify_stretch		  // While respecting constraints stretch items to fill main axis
	};

	// Should probably be in properties, but I don't want to include properties here
	enum flex_direction {
		row,
		row_reversed,
		column,
		column_reversed,
		undefined // Only used internally
	};

	struct flex_flow {
		flex_flow() = default;
		~flex_flow() = default;

		flex_flow(flex_direction axis);
		flex_flow(flex_wrap wrap);
		flex_flow(flex_direction axis, flex_wrap wrap);

		flex_direction main = row;
		flex_direction cross = column;

		flex_wrap wrap = no_wrap;

		flex_align align = align_start;
		flex_justify_content justify_content = justify_start;
	};
}// namespace carbon

#endif