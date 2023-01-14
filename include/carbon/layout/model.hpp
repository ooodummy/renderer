#ifndef _CARBON_LAYOUT_MODEL_HPP_
#define _CARBON_LAYOUT_MODEL_HPP_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace carbon {
	class padded_box {
	public:
		padded_box() = default;
		~padded_box() = default;

		padded_box(float size);
		padded_box(float vertical, float horizontal);
		padded_box(float top, float horizontal, float bottom);
		padded_box(float top, float right, float bottom, float left);

		void set_edge(const glm::vec4& bounds);
		glm::vec2 get_padding() const;

		const glm::vec4& get_edge() const;
		const glm::vec4& get_content() const;

	private:
		float top_ = 0.0f;
		float right_ = 0.0f;
		float bottom_ = 0.0f;
		float left_ = 0.0f;

		glm::vec4 edge_;
		glm::vec4 content_;
	};

	// https://www.w3.org/TR/CSS2/box.html
	class box_model {
	public:
		box_model() = default;
		~box_model() = default;

		bool is_dirty() const;
		virtual void mark_dirty_and_propagate() = 0;

		void compute_box_model();

		const glm::vec2& get_pos() const;
		void set_pos(const glm::vec2& pos);

		const glm::vec2& get_size() const;
		void set_size(const glm::vec2& size);

		const padded_box& get_margin() const;
		void set_margin(const carbon::padded_box& margin);

		const padded_box& get_border() const;
		void set_border(const carbon::padded_box& border);

		const padded_box& get_padding() const;
		void set_padding(const carbon::padded_box& padding);

		glm::vec4 get_bounds() const;
		const glm::vec4& get_content() const;
		glm::vec2 get_total_padding() const;

	protected:
		bool dirty_ = true;

		glm::vec2 pos_{};
		glm::vec2 size_{};

		glm::vec4 content_;

		padded_box margin_;
		padded_box border_;
		padded_box padding_;
	};
}// namespace carbon

#endif