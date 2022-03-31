#ifndef _RENDERER_ENGINE_LAYOUT_HPP_
#define _RENDERER_ENGINE_LAYOUT_HPP_

#include <memory>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace renderer {
    enum flex_direction {
        flex_direction_horizontal,
        flex_direction_vertical
    };

    enum flex_align {
        flex_align_stretch,
        flex_align_start,
        flex_align_end,
        flex_align_center
    };

    enum flex_justify_content {
        flex_justify_start,
        flex_justify_end,
        flex_justify_center,
        flex_justify_space_around,
        flex_justify_space_between,
        flex_justify_space_evenly
    };

    // TODO: Public the enable in all my other usages!
    //  Wrapping will not be handled but shouldn't be needed in the UI except for text.
    //  When labels are drawn with carbon they will have a layout_item that has a impl for getting the glyph bounds
    //  and will automatically add newlines.
    //  Have layout_item be a base class and then make flex_layout and grid_layout
    class layout_item : public std::enable_shared_from_this<layout_item> {
    public:
        layout_item() = default;
        ~layout_item() = default;

        // Would rather define in layout_container
        // but layout_item needs to be marked as polymorphic
        virtual void compute();

        void set_parent(std::shared_ptr<layout_item> item);
        std::shared_ptr<layout_item> get_parent() const;
        std::shared_ptr<layout_item> get_top_parent() const;

        glm::vec2 get_pos() const;
        void set_pos(glm::vec2 pos);

        glm::vec2 get_size() const;
        void set_size(glm::vec2 size);

        glm::vec4 get_bounds() const;

        glm::vec4 get_margin() const;
        void set_margin(glm::vec4 margin);
        void set_margin(glm::vec2 margin);
        void set_margin(float margin);;

        glm::vec4 get_padding() const;
        void set_padding(glm::vec4 padding);
        void set_padding(glm::vec2 padding);
        void set_padding(float padding);

    protected:
        std::shared_ptr<layout_item> parent_;

        glm::vec2 pos_{};
        glm::vec2 size_{};

        // Spacing format will be like this
        // should be self-explanatory without an example
        //      y
        //   +-----+
        // x |     | z
        //   +-----+
        //      w
        glm::vec4 padding_ = { 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 margin_ = {0.0f, 0.0f, 0.0f, 0.0f };
    };

    // All derived one's need flex layout data since a container can be a flexbox item
    class flex_layout_item : public layout_item {
    public:
        glm::vec2 get_min() const;
        void set_min(glm::vec2 min);

        glm::vec2 get_max() const;
        void set_max(glm::vec2 max);

        glm::vec2 get_grow() const;
        void set_grow(glm::vec2 grow);

        glm::vec2 get_shrink() const;
        void set_shrink(glm::vec2 shrink);

    protected:
        // Clamp at the end of scaling, might need to do more intense logic though
        glm::vec2 min_ = { 0.0f, 0.0f };
        glm::vec2 max_ = { FLT_MAX, FLT_MAX };

        // TODO: Grow and shrink probably only need to be floats
        //  since layout items are then used by flex containers
        //  and only go in one direction, but there is a 40% chance
        //  I'm incorrect

        // https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout/Basic_Concepts_of_Flexbox#the_flex-grow_property
        glm::vec2 grow_ = { 0.0f, 0.0f };

        // https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout/Basic_Concepts_of_Flexbox#the_flex-shrink_property
        glm::vec2 shrink_ = { 0.0f, 0.0f };
    };

    class layout_container : public layout_item  {
    public:
        std::shared_ptr<layout_item> add_child(std::shared_ptr<layout_item> item = nullptr);

        std::vector<std::shared_ptr<layout_item>> get_children() const;

    protected:
        std::vector<std::shared_ptr<layout_item>> children_;
    };

    class grid_layout : public layout_container {
    public:
        void compute() override;

        void set_grid(glm::u16vec2 grid);

    private:
        glm::u16vec2 grid_ = { 0, 0 };
    };

    class flex_layout : public layout_container {
    public:
        void compute() override;

        void set_direction(flex_direction direction);
        void set_align(flex_align align);
        void set_justify_context(flex_justify_content justify_content);

    private:
        flex_direction direction_ = flex_direction_horizontal;
        flex_align align_ = flex_align_start; // Is flex align used
        flex_justify_content justify_content_ = flex_justify_start;
    };
}

#endif