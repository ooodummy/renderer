#include "renderer/layout.hpp"

#include <utility>

void renderer::layout_item::compute() {
    // No need to compute anything I just can't have layout_item be a pure virtual
    //  since this could just be an item that has no need of computing any more information
}

void renderer::layout_item::set_parent(std::shared_ptr<layout_item> item) {
    parent_ = std::move(item);
}

std::shared_ptr<renderer::layout_item> renderer::layout_item::get_parent() const {
    return parent_;
}

std::shared_ptr<renderer::layout_item> renderer::layout_item::get_top_parent() const {
    if (!parent_)
        return nullptr;

    for (auto parent = parent_;; parent = parent->get_parent()) {
        if (!parent->get_parent())
            return parent;
    }
}

void renderer::layout_item::set_pos(glm::vec2 pos) {
    pos_ = pos;
}

glm::vec2 renderer::layout_item::get_pos() const {
    return pos_;
}

void renderer::layout_item::set_size(glm::vec2 size) {
    size_ = size;
}

glm::vec2 renderer::layout_item::get_size() const {
    return size_;
}

glm::vec4 renderer::layout_item::get_bounds() const {
    return { pos_, size_ };
}

void renderer::layout_item::set_margin(glm::vec4 margin) {
    margin_ = margin;
}

void renderer::layout_item::set_margin(glm::vec2 margin) {
    margin_ = { margin, margin };
}

void renderer::layout_item::set_margin(float margin) {
    margin_ = {
        margin,
        margin,
        margin,
        margin
    };
}

glm::vec4 renderer::layout_item::get_margin() const {
    return margin_;
}

void renderer::layout_item::set_padding(glm::vec4 padding) {
    padding_ = padding;
}

void renderer::layout_item::set_padding(glm::vec2 padding) {
    padding_ = { padding, padding };
}

void renderer::layout_item::set_padding(float padding) {
    padding_ = {
        padding,
        padding,
        padding,
        padding
    };
}


glm::vec4 renderer::layout_item::get_padding() const {
    return padding_;
}

std::shared_ptr<renderer::layout_item> renderer::layout_container::add_child(std::shared_ptr<layout_item> item) {
    // Create a default item when undefined
    if (item == nullptr)
        item = std::make_shared<renderer::layout_item>();

    item->set_parent(shared_from_this());
    children_.emplace_back(item);
    return item;
}

std::vector<std::shared_ptr<renderer::layout_item>> renderer::layout_container::get_children() const {
    return children_;
}

glm::vec2 renderer::flex_layout_item::get_min() const {
    return min_;
}

void renderer::flex_layout_item::set_min(glm::vec2 min) {
    min_ = min;
}

glm::vec2 renderer::flex_layout_item::get_max() const {
    return max_;
}

void renderer::flex_layout_item::set_max(glm::vec2 max) {
    max_ = max;
}

glm::vec2 renderer::flex_layout_item::get_grow() const {
    return grow_;
}

void renderer::flex_layout_item::set_grow(glm::vec2 grow) {
    grow_ = grow;
}

glm::vec2 renderer::flex_layout_item::get_shrink() const {
    return shrink_;
}

void renderer::flex_layout_item::set_shrink(glm::vec2 shrink) {
    shrink_ = shrink;
}

void renderer::grid_layout::compute() {
    if (children_.size() > grid_.x * grid_.y)
        assert(false);

    const glm::vec2 size = {
        size_.x / static_cast<float>(grid_.x),
        size_.y / static_cast<float>(grid_.y)
    };

    glm::u16vec2 grid_pos{};

    for (auto & child : children_) {
        child->compute();

        const auto margin = child->get_margin();

        glm::vec2 child_pos = {
            pos_.x + size.x * static_cast<float>(grid_pos.x) + margin.x,
            pos_.y + size.y * static_cast<float>(grid_pos.y) + margin.y
        };

        glm::vec2 child_size = {
            size.x - margin.x - margin.z,
            size.y - margin.y - margin.w
        };

        child->set_pos(child_pos);
        child->set_size(child_size);

        // TODO: Setup direction and alignment options for
        //  incrementing the grid position

        // Go to the next grid
        grid_pos.x++;
        if (grid_pos.x >= grid_.x) {
            grid_pos.x = 0;
            grid_pos.y++;
            if (grid_pos.y > grid_.y) {
                // TODO: Options to have grids automatically resize
                //  between width and height

                // Until then assert since this is undefined behavior
                assert(false);
            }
        }
    }
}

void renderer::grid_layout::set_grid(glm::u16vec2 grid) {
    grid_ = grid;
}

void renderer::flex_layout::compute() {
    float grow_space = direction_ == flex_direction_horizontal ? size_.x : size_.y;

    for (auto& child : get_children()) {
        auto flex_child = dynamic_cast<flex_layout_item*>(child.get());
        auto min = flex_child->get_min();

        grow_space -= direction_ == flex_direction_horizontal ? min.x : min.y;
    }

    for (size_t i = 0; i < children_.size(); i++) {

    }
}

void renderer::flex_layout::set_direction(flex_direction direction) {
    direction_ = direction;
}

void renderer::flex_layout::set_align(flex_align align) {
    align_ = align;
}

void renderer::flex_layout::set_justify_context(renderer::flex_justify_content justify_content) {
    justify_content_ = justify_content;
}