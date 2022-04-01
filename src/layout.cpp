#include "carbon/layout.hpp"

#include <algorithm>

#include <glm/glm.hpp>

void carbon::layout_item::compute() {
    // No need to compute anything I just can't have layout_item be a pure virtual
    //  since this could just be an item that has no need of computing any more information
}

void carbon::layout_item::set_parent(std::shared_ptr<layout_item> item) {
    parent_ = std::move(item);
}

std::shared_ptr<carbon::layout_item> carbon::layout_item::get_parent() const {
    return parent_;
}

std::shared_ptr<carbon::layout_item> carbon::layout_item::get_top_parent() const {
    if (!parent_)
        return nullptr;

    for (auto parent = parent_;; parent = parent->get_parent()) {
        if (!parent->get_parent())
            return parent;
    }
}

void carbon::layout_item::set_pos(glm::vec2 pos) {
    pos_ = pos;
}

glm::vec2 carbon::layout_item::get_pos() const {
    return pos_;
}

void carbon::layout_item::set_size(glm::vec2 size) {
    size_ = size;
}

glm::vec2 carbon::layout_item::get_size() const {
    return size_;
}

glm::vec4 carbon::layout_item::get_bounds() const {
    return { pos_, size_ };
}

void carbon::layout_item::set_margin(glm::vec4 margin) {
    margin_ = margin;
}

void carbon::layout_item::set_margin(glm::vec2 margin) {
    margin_ = { margin, margin };
}

void carbon::layout_item::set_margin(float margin) {
    margin_ = {
        margin,
        margin,
        margin,
        margin
    };
}

glm::vec4 carbon::layout_item::get_margin() const {
    return margin_;
}

void carbon::layout_item::set_padding(glm::vec4 padding) {
    padding_ = padding;
}

void carbon::layout_item::set_padding(glm::vec2 padding) {
    padding_ = { padding, padding };
}

void carbon::layout_item::set_padding(float padding) {
    padding_ = {
        padding,
        padding,
        padding,
        padding
    };
}


glm::vec4 carbon::layout_item::get_padding() const {
    return padding_;
}

std::shared_ptr<carbon::flex_item> carbon::flex_item::add_child(std::shared_ptr<flex_item> item) {
    item->set_parent(shared_from_this());
    children_.emplace_back(item);
    return item;
}

std::vector<std::shared_ptr<carbon::flex_item>> carbon::flex_item::get_children() const {
    return children_;
}

std::shared_ptr<carbon::flex_item> carbon::flex_item::add_flex_item() {
    auto item = std::make_shared<flex_item>();
    item->set_parent(shared_from_this());
    children_.emplace_back(item);
    return item;
}

std::shared_ptr<carbon::grid_container> carbon::flex_item::add_grid_container() {
    auto item = std::make_shared<grid_container>();
    item->set_parent(shared_from_this());
    children_.emplace_back(item);
    return item;
}

std::shared_ptr<carbon::flex_container> carbon::flex_item::add_flex_container() {
    auto item = std::make_shared<flex_container>();
    item->set_parent(shared_from_this());
    children_.emplace_back(item);
    return item;
}

glm::vec2 carbon::flex_item::get_min() const {
    return min_;
}

void carbon::flex_item::set_min(glm::vec2 min) {
    min_ = min;
}

glm::vec2 carbon::flex_item::get_max() const {
    return max_;
}

void carbon::flex_item::set_max(glm::vec2 max) {
    max_ = max;
}

float carbon::flex_item::get_grow() const {
    return grow_;
}

void carbon::flex_item::set_grow(float grow) {
    grow_ = grow;
}

glm::vec2 carbon::flex_item::get_shrink() const {
    return shrink_;
}

void carbon::flex_item::set_shrink(glm::vec2 shrink) {
    shrink_ = shrink;
}

void carbon::grid_container::compute() {
    if (children_.size() > grid_.x * grid_.y)
        assert(false);

    const glm::vec2 size = {
        size_.x / static_cast<float>(grid_.x),
        size_.y / static_cast<float>(grid_.y)
    };

    glm::i16vec2 grid_pos = get_grid_start();

    for (auto & child : children_) {
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

        child->compute();

        increment_and_resize_grid(grid_pos);
    }
}

void carbon::grid_container::set_grid(glm::i16vec2 grid) {
    grid_ = grid;
}

void carbon::grid_container::set_column_direction(flex_direction direction) {
    column_direction_ = direction;
}

void carbon::grid_container::set_row_direction(flex_direction direction) {
    row_direction_ = direction;
}

glm::i16vec2 carbon::grid_container::get_grid_start() {
    glm::u16vec2 start{};

    if (row_direction_ == flex_direction_row_reverse)
        start.x = grid_.x - 1;

    if (column_direction_ == flex_direction_column_reverse)
        start.y = grid_.y - 1;

    return start;
}

void carbon::grid_container::increment_and_resize_grid(glm::i16vec2& pos) {
    // TODO: Handle resizing grid

    if (row_direction_ == flex_direction_row)
        pos.x++;
    else
        pos.x--;

    if ((row_direction_ == flex_direction_row && pos.x >= grid_.x) ||
        (row_direction_ == flex_direction_row_reverse && pos.x < 0)) {
        // This feels like this call is unnecessary
        const auto start = get_grid_start();
        pos.x = start.x;

        if (column_direction_ == flex_direction_column)
            pos.y++;
        else
            pos.y--;

        if ((column_direction_ == flex_direction_row && pos.y >= grid_.y) ||
            (column_direction_ == flex_direction_row_reverse && pos.y < 0)) {
            assert(false);
        }
    }
}

void carbon::grid_container::set_resize(carbon::flex_grid_resize resize) {
    resize_ = resize;
}

void carbon::flex_container::compute() {
    cross_axis_ = get_cross_axis();

    const auto main_size = get_main(size_);
    const auto cross_size = get_cross(size_);

    const auto padding_axes = get_axes(padding_);

    const auto main_size_padded = main_size - padding_axes.x - padding_axes.y;
    const auto cross_size_padded = cross_size - padding_axes.z - padding_axes.w;

    // Subtract minimum sizes of children's main axis min size
    size_t grow_count = 0;
    auto max_grow_remaining = main_size_padded;

    for (auto& child: children_) {
        if (child->get_grow() > 0.0f)
            grow_count++;

        max_grow_remaining -= get_main(child->get_min());
    }

    const auto grow_size = max_grow_remaining / static_cast<float>(grow_count);

    auto pos_axes = get_axes(pos_);
    pos_axes.x += padding_axes.x;
    pos_axes.y += padding_axes.y;

    for (auto& child: children_) {
        const auto min = child->get_min();
        const auto max = child->get_max();

        const auto margin_axes = get_axes(child->get_margin());
        const auto margin_axes_sum = get_sum(margin_axes);

        glm::vec2 size_axes = {
            get_main(min),
            cross_size_padded
        };

        const auto grow = child->get_grow();

        if (grow > 0.0f)
            size_axes.x += (grow * grow_size);

        size_axes.x -= margin_axes_sum.x;
        size_axes.y -= margin_axes_sum.y;

        glm::vec2 child_size;
        set_axes(child_size, size_axes);

        child_size.x = std::clamp(child_size.x, min.x, max.x);
        child_size.y = std::clamp(child_size.y, min.y, max.y);

        child->set_size(child_size);

        // Position
        pos_axes.x += margin_axes.x;
        pos_axes.y += margin_axes.z;

        glm::vec2 child_pos;
        set_axes(child_pos, pos_axes);

        pos_axes.x += size_axes.x + margin_axes.y;
        pos_axes.y -= margin_axes.z;

        child->set_pos(child_pos);

        child->compute();
    }
}

void carbon::flex_container::set_direction(flex_direction direction) {
    main_axis_ = direction;
}

void carbon::flex_container::set_align(flex_align align) {
    align_ = align;
}

void carbon::flex_container::set_justify_context(flex_justify_content justify_content) {
    justify_content_ = justify_content;
}

float carbon::flex_container::get_sum(glm::vec2 src) {
    return src.x + src.y;
}

glm::vec2 carbon::flex_container::get_sum(glm::vec4 src) {
    return {src.x + src.y, src.z + src.w};
}

carbon::flex_direction carbon::flex_container::get_cross_axis() {
    switch (main_axis_) {
        case flex_direction_row:
        case flex_direction_row_reverse:
            return flex_direction_column;
        case flex_direction_column:
        case flex_direction_column_reverse:
            return flex_direction_row;
        default:
            assert(false);
            break;
    }

    return flex_direction_column;
}

float carbon::flex_container::get_axis(carbon::flex_direction axis, glm::vec2 src) {
    switch (axis) {
        case flex_direction_row:
        case flex_direction_row_reverse:
            return src.x;
        case flex_direction_column:
        case flex_direction_column_reverse:
            return src.y;
        default:
            assert(false);
            break;
    }

    return 0.0f;
}

glm::vec2 carbon::flex_container::get_axis(carbon::flex_direction axis, glm::vec4 src) {
    switch (axis) {
        case flex_direction_row:
        case flex_direction_row_reverse:
            return { src.x, src.z };
        case flex_direction_column:
        case flex_direction_column_reverse:
            return { src.y, src.w };
        default:
            assert(false);
            break;
    }

    return {};
}

void carbon::flex_container::set_axis(flex_direction axis, glm::vec2& dst, float src) {
    switch (axis) {
        case flex_direction_row:
        case flex_direction_row_reverse:
            dst.x = src;
            break;
        case flex_direction_column:
        case flex_direction_column_reverse:
            dst.y = src;
            break;
        default:
            assert(false);
            break;
    }
}

void carbon::flex_container::set_axis(flex_direction axis, glm::vec4& dst, glm::vec2 src) {
    switch (axis) {
        case flex_direction_row:
        case flex_direction_row_reverse:
            dst.x = src.x;
            dst.z = src.y;
            break;
        case flex_direction_column:
        case flex_direction_column_reverse:
            dst.y = src.y;
            dst.w = src.y;
            break;
        default:
            assert(false);
            break;
    }
}

float carbon::flex_container::get_main(glm::vec2 src) const {
    return get_axis(main_axis_, src);
}

glm::vec2 carbon::flex_container::get_main(glm::vec4 src) const {
    return get_axis(main_axis_, src);
}

void carbon::flex_container::set_main(glm::vec2& dst, float src) const {
    set_axis(main_axis_, dst, src);
}

void carbon::flex_container::set_main(glm::vec4& dst, glm::vec2 src) const {
    set_axis(main_axis_, dst, src);
}

float carbon::flex_container::get_cross(glm::vec2 src) const {
    return get_axis(cross_axis_, src);
}

glm::vec2 carbon::flex_container::get_cross(glm::vec4 src) const {
    return get_axis(cross_axis_, src);
}

void carbon::flex_container::set_cross(glm::vec2& dst, float src) const {
    set_axis(cross_axis_, dst, src);
}

void carbon::flex_container::set_cross(glm::vec4& dst, glm::vec2 src) const {
    set_axis(cross_axis_, dst, src);
}

glm::vec2 carbon::flex_container::get_axes(glm::vec2 dst) const {
    auto main = get_main(dst);
    auto cross = get_cross(dst);

    return {main, cross};
}

glm::vec4 carbon::flex_container::get_axes(glm::vec4 dst) const {
    auto main = get_main(dst);
    auto cross = get_cross(dst);

    return {main, cross};
}

void carbon::flex_container::set_axes(glm::vec2& dst, glm::vec2 src) const {
    set_main(dst, src.x);
    set_cross(dst, src.y);
}

void carbon::flex_container::set_axes(glm::vec4& dst, glm::vec4 src) const {
    set_main(dst, {src.x, src.y});
    set_cross(dst, {src.z, src.w});
}
