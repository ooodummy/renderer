#ifndef _CARBON_LAYOUT_MODEL_HPP_
#define _CARBON_LAYOUT_MODEL_HPP_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace carbon {
	struct padded_box {
		padded_box() = default;
		~padded_box() = default;

		padded_box(float size); // NOLINT(google-explicit-constructor)
		padded_box(float vertical, float horizontal);
		padded_box(float top, float horizontal, float bottom);
		padded_box(float top, float right, float bottom, float left);

		void compute(const glm::vec4& bounds);

		[[nodiscard]] float get_spacing_width() const;
		[[nodiscard]] float get_spacing_height() const;

		float top = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;
		float left = 0.0f;

		glm::vec4 padded_bounds;
	};

	// https://i.imgur.com/S3nJWLV.png
	class box_model {
	public:
		box_model() = default;
		~box_model() = default;

		void compute_alignment();

		glm::vec2 pos{};
		glm::vec2 size{};

		glm::vec4 bounds;

		padded_box margin{5.0f};
		padded_box border;
		padded_box padding;

		glm::vec4 content_bounds;
	};
}

#endif