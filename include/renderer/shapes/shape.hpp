#ifndef RENDERER_SHAPES_SHAPE_HPP
#define RENDERER_SHAPES_SHAPE_HPP

#include "renderer/vertex.hpp"

#include <d3d11.h>

// https://github.com/applesthepi/unnamedblocks/tree/master/ub_macchiato/include/macchiato/shapes
// Create an object for every shape, so we can cache its vertices and then have options to translate it using a matrix.

namespace renderer {
    // TODO: Compare best ways to actually cache shapes and setup a matrix for transformations without modifying the
    //  vertices
    class shape {
        friend class buffer;

        void check_recalculation();

    protected:
        // Should not need to be done manually
        virtual void recalculate_buffer() = 0;

        bool needs_recalculate_ = true;

        vertex *vertices_;
        size_t vertex_count_;

        color_rgba col_;

        D3D_PRIMITIVE_TOPOLOGY type_;
        ID3D11ShaderResourceView *srv_;
    };
}// namespace renderer

#endif