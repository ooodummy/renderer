#ifndef _CARBON_LAYOUT_ITEM_HPP_
#define _CARBON_LAYOUT_ITEM_HPP_

#include "axis.hpp"
#include "model.hpp"

#include <vector>
#include <memory>

#include <glm/glm.hpp>

namespace carbon {
	enum flex_unit {
		unit_pixel,
		unit_aspect,
		unit_relative,
		unit_auto
	};

	enum flex_basis_size {
		flex_basis_width,
		flex_basis_percentage,
		flex_basis_auto,
		flex_basis_content,
		flex_basis_fit_content,
		flex_basis_max_content,
		flex_basis_min_content
	};

	class flex_item;

	class flex_value {
	public:
		flex_value() = default;
		explicit flex_value(float value);
		explicit flex_value(flex_unit unit);
		flex_value(float value, flex_unit unit);
		flex_value(float value, flex_item* relative);
		~flex_value() = default;

		flex_unit unit;
		float value;

		flex_item* relative;
	};

	// TODO: Later switch to this
	class flex {
	public:
		flex(float grow, float shrink = 1.0f, flex_value basis = flex_value(0.0f));

		float grow;
		float shrink;
		flex_value basis;
	};

	// Ignore the getter and setter abuse I like how it then allows me to interact with the objects
	class flex_item : public box_model {
	public:
		flex_item() = default;
		~flex_item() = default;

		virtual void compute();

		// Used later for widget inheritance
		// just easier to define here
		virtual void draw();
		virtual void input();

		virtual void draw_contents();

		void set_parent(flex_item* item);
		[[nodiscard]] flex_item* get_parent() const;
		[[nodiscard]] flex_item* get_top_parent() const;

		[[nodiscard]] float get_min_width() const;
		void set_min_width(float min_width);

		[[nodiscard]] float get_max_width() const;
		void set_max_width(float max_width);

		[[nodiscard]] float get_grow() const;
		void set_grow(float grow);

		[[nodiscard]] float get_shrink() const;
		void set_shrink(float shrink);

		[[nodiscard]] bool get_basis_auto() const;
		void set_basis_auto(bool inherit);

		// Minimum content size used for basis_ size
		[[nodiscard]] glm::vec2 get_basis_content() const;
		void set_basis_content(glm::vec2 content);

		[[nodiscard]] flex_unit get_basis_unit() const;
		void set_basis_unit(flex_unit unit);

		[[nodiscard]] float get_basis() const;
		void set_basis(float value);

		[[nodiscard]] flex_item* get_basis_relative_item() const;
		void set_basis_relative_item(flex_item* item);

		// Data used for compute
		// Can be put somewhere else later
		float base_size{};
		float hypothetical_size{};
		float final_size{};
		bool clamped = false;

	protected:
		flex_item* parent_ = nullptr;

		float grow_;
		float shrink_;

		bool basis_auto_;
		glm::vec2 basis_content_{};
		flex_value basis_ = flex_value{ unit_aspect };

		float min_width_ = 0.0f;
		float max_width_ = FLT_MAX;
	};
}

#endif