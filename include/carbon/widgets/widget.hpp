#ifndef _CARBON_WIDGETS_WIDGET_HPP_
#define _CARBON_WIDGETS_WIDGET_HPP_

#include "../global.hpp"
#include "../layout.hpp"

#include <memory>

namespace carbon {
    class widget : public std::enable_shared_from_this<widget> {
    public:
        widget() = default;
        ~widget() = default;

        virtual void apply_layout() {}
        virtual void draw() {}
        virtual void input() {}
    };

    class widget_item : public widget, public flex_item {

    };

    class widget_container : public widget, public flex_container {

    };
}

#endif