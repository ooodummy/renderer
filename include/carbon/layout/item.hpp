#ifndef _CARBON_LAYOUT_ITEM_HPP_
#define _CARBON_LAYOUT_ITEM_HPP_

#include "model.hpp"

#include <vector>
#include <memory>

#include <glm/glm.hpp>

namespace carbon {
	enum flex_axis {
		flex_axis_row,
		flex_axis_column
	};

	enum flex_direction {
		flex_direction_forward,
		flex_direction_reversed
	};

	enum size_unit {
		unit_pixel,
		unit_aspect,
		unit_relative
	};

	class flex_item;

	struct size_value {
		size_unit unit = unit_aspect;
		float value = 0.0f;
		flex_item* relative = nullptr;
	};

	struct flex_basis {
		explicit flex_basis(bool inherit = true) : inherit(inherit) {}

		bool inherit;
		glm::vec2 content{};
		size_value size;
	};

	struct flex {
		explicit flex(float grow = 0.0f, float shrink = 1.0f, flex_basis basis = flex_basis(true)) : grow(grow), shrink(shrink), basis(basis) {}

		float grow;
		float shrink;
		flex_basis basis;
	};

	// Ignore the getter and setter abuse I like how it then allows me to interact with the objects
	class flex_item : public box_model {
	public:
		flex_item() = default;
		~flex_item() = default;

		virtual void compute();

		// Used later for widget inheritance
		virtual void draw();
		virtual void input();

		void draw_contents();

		void set_parent(flex_item* item);
		[[nodiscard]] flex_item* get_parent() const;
		[[nodiscard]] flex_item* get_top_parent() const;

		template<typename T, typename... Args>
		T* add_child(Args&&... args) {
			return reinterpret_cast<T*>(add_child(std::unique_ptr<T>(new T(std::forward<Args>(args)...))));
		}

		carbon::flex_item* add_child(std::unique_ptr<flex_item> item);
		[[nodiscard]] std::vector<std::unique_ptr<flex_item>>& get_children();

		[[nodiscard]] glm::vec2 get_min() const;
		void set_min(glm::vec2 min);

		[[nodiscard]] glm::vec2 get_max() const;
		void set_max(glm::vec2 min);

		[[nodiscard]] float get_grow() const;
		void set_grow(float grow);

		[[nodiscard]] float get_shrink() const;
		void set_shrink(float shrink);

		[[nodiscard]] bool get_basis_auto() const;
		void set_basis_auto(bool inherit);

		// Minimum content size used for basis size
		[[nodiscard]] glm::vec2 get_basis_content() const;
		void set_basis_content(glm::vec2 content);

		[[nodiscard]] size_unit get_basis_unit() const;
		void set_basis_unit(size_unit unit);

		[[nodiscard]] float get_basis() const;
		void set_basis(float value);

		[[nodiscard]] flex_item* get_basis_relative_item() const;
		void set_basis_relative_item(flex_item* item);

		// Data used for compute
		float base_size;
		float main_size;

	protected:
		flex_item* parent_ = nullptr;
		std::vector<std::unique_ptr<flex_item>> children_;

		flex flex_;

		glm::vec2 min_{};
		glm::vec2 max_ = { FLT_MAX, FLT_MAX };
	};
}

#endif