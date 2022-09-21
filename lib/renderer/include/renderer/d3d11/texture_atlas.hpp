#ifndef _RENDERER_D3D11_TEXTURE_ATLAS_HPP_
#define _RENDERER_D3D11_TEXTURE_ATLAS_HPP_

#include "texture2d.hpp"

#include <glm/vec4.hpp>

#include <memory>
#include <vector>

namespace renderer {
	class texture_atlas {
	private:
		std::unique_ptr<texture2d> texture;
		std::vector<glm::vec4> textures;

	public:
		//size_t add_texture(texture2d* texture);
	};
}

#endif