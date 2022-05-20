#ifndef _RENDERER_UTIL_BEZIER_HPP_
#define _RENDERER_UTIL_BEZIER_HPP_

#include "easing.hpp"

#include <array>

// https://www.youtube.com/watch?v=aVwxzDHniEw
namespace renderer {
	static std::map<float, float[4]> weight_map;

	size_t binomial(size_t n, size_t k) {
		assert(k <= n);
		size_t val = 1;

		for (size_t i = 1; i <= k; i++) {
			val *= n + 1 - i;
			val /= i;
		}

		return val;
	}

    template <size_t N>
	class binomial_coefficients {
	public:
		binomial_coefficients() {
			size_t center = N / 2;
			size_t k = 0;

			while (k <= center) {
				coefficients_[k] = binomial(N, k);
				k++;
			}

			while (k <= N) {
				coefficients_[k] = coefficients_[N - k];
				k++;
			}
		}

		static constexpr size_t size() {
			return N + 1;
		}

		[[nodiscard]] size_t operator [](size_t idx) const {
			assert(idx < size());
			return coefficients_[idx];
		}

	private:
		size_t coefficients_[size()]{};
	};

	struct polynomial_pair {
		size_t t = 0;
		size_t one_minus_t = 0;

		[[nodiscard]] float at(float t) const {
			return float(pow(1.0f - t, one_minus_t) * pow(t, float(this->t)));
		}
	};

	template<size_t N>
	class polynomial_coefficients {
	public:
		polynomial_coefficients() {
			for (size_t i = 0; i <= N; i++) {
				pairs_[i].t = i;
				pairs_[i].one_minus_t = N - i;

				assert(pairs_[i].t + pairs_[i].one_minus_t == N);
			}
		}

		[[nodiscard]] float at(size_t pos, float t) const {
			assert(pos < size());
			return pairs_[pos].at(t);
		}

		static constexpr size_t size() {
			return N + 1;
		}

		const polynomial_pair& operator [](size_t idx) const {
			assert(idx < size());
			return pairs_[idx];
		}

	private:
		polynomial_pair pairs_[size()];
	};

	struct bezier_point {
		glm::vec2 pos;
		glm::vec2 vel;
		float acc;
		float curvature;
	};

	template <size_t N>
	class bezier_line {
		bezier_point points[N];
		glm::vec2 jerk;
	};

	template <size_t N>
	class bezier_curve {
	public:
		bezier_curve() = default;

		explicit bezier_curve(const std::vector<glm::vec2>& control_points) {
			assert(control_points.size() == size());
			for (size_t i = 0; i < control_points.size(); i++)
				control_points_[i] = control_points[i];
		}

		[[nodiscard]] size_t order() const {
			return N;
		}

		[[nodiscard]] size_t size() const {
			return N + 1;
		}

		bezier_curve<N-1> derivative() const {
			assert(N != 0);

			std::vector<glm::vec2> derivative_weights(N);
			for (size_t i = 0; i < N; i++) {
				const glm::vec2 weight = {
					(control_points_[i + 1].x - control_points_[i].x) * N,
					(control_points_[i + 1].y - control_points_[i].y) * N
				};

				derivative_weights[i] = weight;
			}

			return bezier_curve<N-1>(derivative_weights);
		}

		[[nodiscard]] float at(float t, size_t axis) const {
			assert(axis < 2);
			float sum = 0.0f;
			for (size_t n = 0; n < N+1; n++) {
				sum += binomial_coefficients[n] * polynomial_coefficients[n].at(t) * control_points_[n][axis];
			}
			return sum;
		}

		[[nodiscard]] glm::vec2 at(float t) const {
			glm::vec2 p;
			for (size_t i = 0; i < 2; i++)
			{
				p[i] = at(t, i);
			}
			return p;
		}

		[[nodiscard]] glm::vec2 tangent_at(float t, bool normalize = true) const
		{
			glm::vec2 p;
			bezier_curve<N-1> derivative = this->derivative();
			p = derivative.at(t);
			if (normalize)
				return glm::normalize(p);
			return p;
		}

		glm::vec2& operator[](size_t idx)
		{
			assert(idx < size());
			return control_points_[idx];
		}

		glm::vec2 operator[](size_t idx) const
		{
			assert(idx < size());
			return control_points_[idx];
		}

		static const binomial_coefficients<N> binomial_coefficients;
		static const polynomial_coefficients<N> polynomial_coefficients;

	private:
		std::array<glm::vec2, N + 1> control_points_;
	};

	template<size_t N>
	const binomial_coefficients<N> bezier_curve<N>::binomial_coefficients = renderer::binomial_coefficients<N>();

	template<size_t N>
	const polynomial_coefficients<N> bezier_curve<N>::polynomial_coefficients = renderer::polynomial_coefficients<N>();
}

#endif