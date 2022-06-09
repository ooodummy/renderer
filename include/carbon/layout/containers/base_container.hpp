#ifndef _CARBON_LAYOUT_CONTAINERS_BASE_CONTAINER_HPP_
#define _CARBON_LAYOUT_CONTAINERS_BASE_CONTAINER_HPP_

#include "../item.hpp"

#include <memory>
#include <vector>

namespace carbon {
	class base_container : public flex_item {
	public:
		void draw() override;

		template<typename T, typename... Args>
		T* add_child(Args&&... args) {
			return reinterpret_cast<T*>(add_child(std::unique_ptr<T>(new T(std::forward<Args>(args)...))));
		}

		carbon::flex_item* add_child(std::unique_ptr<flex_item> item);
		std::vector<std::unique_ptr<flex_item>>& get_children();

	protected:
		std::vector<std::unique_ptr<flex_item>> children_;
	};
}

#endif