#ifndef _CARBON_WIDGETS_WINDOW_HPP_
#define _CARBON_WIDGETS_WINDOW_HPP_

#include "widget.hpp"

namespace carbon {
    class title_bar : public widget_container {
    public:
        void apply_layout() override {
            set_min({0.0f, 38.0f});
        }

        void draw() override {
            buf->draw_rect_filled(get_bounds(), COLOR_PURPLE);
        }
    };

    class tab_bar : public widget_container {
    public:
        void apply_layout() override {
            set_min({120.0f, 0.0f});
            set_direction(carbon::flex_direction_column);
        }

        void draw() override {
            buf->draw_rect_filled(get_bounds(), COLOR_GREEN);
        }
    };

    class sub_tab_bar : public widget_container {
    public:
        void apply_layout() override {
            set_min({0.0f, 28.0f});
        }

        void draw() override {
            buf->draw_rect_filled(get_bounds(), COLOR_BLUE);
        }
    };

    class window : public widget_container {
    public:
        window() {
            title_bar_ = std::make_shared<title_bar>();
            container1_ = std::make_shared<flex_container>();
            tab_bar_ = std::make_shared<tab_bar>();
            container2_ = std::make_shared<flex_container>();
            sub_tab_bar_ = std::make_shared<sub_tab_bar>();
        }

        void apply_layout() override {
            set_pos({150.0f, 150.0f});
            set_size({580.0f, 500.0f});
            set_direction(carbon::flex_direction_column);

            add_child(title_bar_);
            title_bar_->apply_layout();

            add_child(container1_);
            container1_->set_grow(1.0f);

            container1_->add_child(tab_bar_);
            tab_bar_->apply_layout();

            container1_->add_child(container2_);
            container2_->set_grow(1.0f);
            container2_->set_direction(carbon::flex_direction_column);

            container2_->add_child(sub_tab_bar_);
            sub_tab_bar_->apply_layout();

            container2_->add_child(holder_);
            holder_->set_grow(1.0f);
            holder_->set_margin(10.0f);
        }

        void draw() override {
            const auto bounds = get_bounds();

            // Outline
            buf->draw_rect(bounds + glm::vec4(-1.0f, -1.0f, 2.0f, 2.0f), COLOR_WHITE);

            title_bar_->draw();
            tab_bar_->draw();
            sub_tab_bar_->draw();

            /*auto draw_impl = [](const std::shared_ptr<widget>& widget, auto& draw_ref) -> void { // NOLINT(misc-no-recursion)
                for (auto& child : widget->get_children()) {
                    // Call widget draw

                    draw_ref(child, draw_ref);
                }
            };

            draw_impl(flex_item, draw_impl);*/
        }

    private:
        std::shared_ptr<title_bar> title_bar_ = nullptr;
        std::shared_ptr<flex_container> container1_;
        std::shared_ptr<tab_bar> tab_bar_;
        std::shared_ptr<flex_container> container2_;
        std::shared_ptr<sub_tab_bar> sub_tab_bar_;
        std::shared_ptr<flex_container> holder_;
    };
}

#endif