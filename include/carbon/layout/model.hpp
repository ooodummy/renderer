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

		[[nodiscard]] glm::vec4 get_bounds() const;
		operator glm::vec4() const; // NOLINT(google-explicit-constructor)

		[[nodiscard]] float get_width() const;
		[[nodiscard]] float get_height() const;

		float top;
		float right;
		float bottom;
		float left;
	};

	// https://i.imgur.com/S3nJWLV.png
	class box_model {
	public:
		box_model() = default;
		~box_model() = default;

		[[nodiscard]] glm::vec2 get_pos() const;
		void set_pos(glm::vec2 pos);

		[[nodiscard]] glm::vec2 get_size() const;
		void set_size(glm::vec2 size);

		[[nodiscard]] glm::vec4 get_bounds() const;

		box_alignment get_margin() const;
		void set_margin(box_alignment margin);

		box_alignment get_border() const;
		void set_border(box_alignment border);

		box_alignment get_padding() const;
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