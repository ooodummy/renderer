#include "carbon/layout/properties.hpp"

carbon::flex_width::flex_width(float value) : value(value), unit(unit_aspect) {}
carbon::flex_width::flex_width(carbon::flex_unit unit) : unit(unit), value(0.0f) {}
carbon::flex_width::flex_width(float value, flex_unit unit) : value(value), unit(unit) {}
// carbon::flex_width::flex_width(float value, carbon::flex_item* relative) : value(value), relative(relative),
// unit(unit_relative) {}

carbon::flex_basis::flex_basis(float value) : width(value) {}
carbon::flex_basis::flex_basis(flex_unit unit) : width(unit) {}
carbon::flex_basis::flex_basis(float value, flex_unit unit) : width(value, unit) {}
// carbon::flex_basis::flex_basis(float value, carbon::flex_item* relative) : width(value, relative) {}
carbon::flex_basis::flex_basis(bool minimum) : minimum(minimum) {}

carbon::flex::flex(float grow) : grow(grow) {}
carbon::flex::flex(float grow, float shrink) : grow(grow), shrink(shrink) {}
carbon::flex::flex(float grow, float shrink, flex_basis basis) : grow(grow), shrink(shrink), basis(basis) {}
carbon::flex::flex(flex_basis basis) : basis(basis) {}
carbon::flex::flex(float grow, flex_basis basis) : grow(grow), basis(basis) {}
carbon::flex::flex(flex_keyword_values keyword) {
	basis.minimum = true;
	switch (keyword) {
		case value_initial:
			grow = 0.0f;
			shrink = 1.0f;
			break;
		case value_auto:
			grow = 1.0f;
			shrink = 1.0f;
			break;
		case value_none:
			grow = 0.0f;
			shrink = 0.0f;
			break;
	}
}

carbon::flex_flow::flex_flow(carbon::flex_direction axis) :
	main(axis),
	cross((axis == row || axis == row_reversed) ? column : row) {}
carbon::flex_flow::flex_flow(carbon::flex_wrap wrap) : wrap(wrap) {}
carbon::flex_flow::flex_flow(carbon::flex_direction axis, carbon::flex_wrap wrap) :
	main(axis),
	cross((axis == row || axis == row_reversed) ? column : row),
	wrap(wrap) {}