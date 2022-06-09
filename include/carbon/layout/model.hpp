#ifndef _CARBON_LAYOUT_MODEL_HPP_
#define _CARBON_LAYOUT_MODEL_HPP_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace carbon {
	struct padded_box {
		padded_box() = default;
		~padded_box() = default;

		padded_box(float size);
		padded_box(float vertical, float horizontal);
		padded_box(float top, float horizontal, float bottom);
		padded_box(float top, float right, float bottom, float left);

		void compute(const glm::vec4& bounds);

		[[nodiscard]] glm::vec2 get_padding() const;

		[[nodiscard]] float get_padding_width() const;
		[[nodiscard]] float get_padding_height() const;

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

		friend class grid_container;
		friend class flex_container;
		
		virtual void mark_dirty_and_propagate() = 0;

		[[nodiscard]] const glm::vec2& get_pos() const;
		void set_pos(const glm::vec2& pos);

		[[nodiscard]] const glm::vec2& get_size() const;
		void set_size(const glm::vec2& size);

		[[nodiscard]] const glm::vec4& get_content_bounds() const;

		void compute_alignment();
		[[nodiscard]] glm::vec2 get_total_padding() const;
		[[nodiscard]] glm::vec2 get_total_border_padding();

		[[nodiscard]] const padded_box& get_margin() const;
		void set_margin(const carbon::padded_box& margin);

		[[nodiscard]] const padded_box& get_border() const;
		void set_border(const carbon::padded_box& border);

		[[nodiscard]] const padded_box& get_padding() const;
		void set_padding(const carbon::padded_box& padding);

	protected:
		bool dirty_ = true;

		glm::vec2 pos_{};
		glm::vec2 size_{};

		glm::vec4 bounds_;
		glm::vec4 content_bounds_;

		padded_box margin_{3.0f};
		padded_box border_{3.0f};
		padded_box padding_{3.0f};
	};
}

#endif