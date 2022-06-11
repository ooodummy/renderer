#include "renderer/geometry/bezier.hpp"

size_t renderer::binomial(size_t n, size_t k) {
	assert(k <= n);
	size_t val = 1;

	for (size_t i = 1; i <= k; i++) {
		val *= n + 1 - i;
		val /= i;
	}

	return val;
}

float renderer::polynomial_pair::at(float t) const {
	return float(pow(1.0f - t, one_minus_t) * pow(t, float(this->t)));
}
