#include "carbon/carbon.hpp"

void carbon::init() {
	init_default_theme();
}

void carbon::begin() {
	benchmark.flex_compute_calls = 0;
}

void carbon::end() {
	input_end();
}
