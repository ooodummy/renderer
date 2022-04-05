#ifndef _CARBON_LAYOUT_MODEL_HPP_
#define _CARBON_LAYOUT_MODEL_HPP_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace carbon {
	struct box_alignment {
		box_alignment() = default;
		~box_alignment() = default;

		box_alignment(float size); // NOLINT(google-explicit-constructor)
		box_alignment(float vertical, float horizontal);
		box_alignment(float top, float horizontal, float bottom);
		box_alignment(float top, float right, float bottom, float left);

		void compute(const glm::vec4& container);

		[[nodiscard]] glm::vec4 get_alignment() const;

		[[nodiscard]] float get_alignment_width() const;
		[[nodiscard]] float get_alignment_height() const;

		float top;
		float right;
		float bottom;
		float left;

		[[nodiscard]] glm::vec4 get_bounds() const;
		[[nodiscard]] glm::vec4 get_inner_bounds() const;

		glm::vec2 pos;
		glm::vec2 size;

		glm::vec2 inner_pos;
		glm::vec2 inner_size;
	};

	// https://i.imgur.com/S3nJWLV.png
	class box_model {
	public:
		box_model() = default;
		~box_model() = default;

		void compute_alignment();

		[[nodiscard]] glm::vec2 get_pos() const;
		void set_pos(glm::vec2 pos);

		[[nodiscard]] glm::vec2 get_size() const;
		void set_size(glm::vec2 size);

		[[nodiscard]] glm::vec4 get_bounds() const;

		[[nodiscard]] glm::vec2 get_content_pos() const;
		[[nodiscard]] glm::vec2 get_content_size() const;
		[[nodiscard]] glm::vec4 get_content_bounds() const;

		[[nodiscard]] box_alignment get_margin() const;
		void set_margin(box_alignment margin);

		[[nodiscard]] box_alignment get_border() const;
		void set_border(box_alignment border);

		[[nodiscard]] box_alignment get_padding() const;
		void set_padding(box_alignment padding);

	protected:
		glm::vec2 pos_{};
		glm::vec2 size_{};

		box_alignment margin_{};
		box_alignment border_{};
		box_alignment padding_{};

		glm::vec4 content_bounds_{};
	};
}

#endif