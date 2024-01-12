#ifndef RENDERER_D3D11_SHADERS_CONSTANT_BUFFERS_HPP
#define RENDERER_D3D11_SHADERS_CONSTANT_BUFFERS_HPP

#include <glm/glm.hpp>

namespace renderer {
	struct alignas(16) global_buffer {
		glm::vec2 dimensions;
	};

	struct alignas(64) command_buffer {
		glm::vec4 scissor_bounds;
		glm::vec4 key_color;
		float blur_strength;
		uint32_t scissor_enable;
		uint32_t scissor_in;
		uint32_t scissor_circle;
		uint32_t key_enable;
		uint32_t is_texture;
		uint32_t is_mask;
	};
}// namespace renderer

#endif