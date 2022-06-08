#ifndef _CARBON_LAYOUT_CONTAINERS_BASE_HPP_
#define _CARBON_LAYOUT_CONTAINERS_BASE_HPP_

#include "../axes.hpp"
#include "../item.hpp"

#include <memory>
#include <vector>

namespace carbon {
	class base_container : public flex_item {
	public:
		void draw_contents() override;

		template<typename T, typename... Args>
		T* add_child(Args&&... args) {
			return reinterpret_cast<T*>(add_child(std::unique_ptr<T>(new T(std::forward<Args>(args)...))));
		}

		carbon::flex_item* add_child(std::unique_ptr<flex_item> item);
		[[nodiscard]] std::vector<std::unique_ptr<flex_item>>& get_children();

	protected:
		std::vector<std::unique_ptr<flex_item>> children_;
	};
}

#endif