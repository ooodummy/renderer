#include "renderer/shapes/shape.hpp"

void renderer::shape::check_recalculation() {
	if (needs_recalculate_)
		recalculate_buffer();
}