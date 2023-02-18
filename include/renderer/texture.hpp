#ifndef RENDERER_TEXTURE2D_HPP
#define RENDERER_TEXTURE2D_HPP

#include <map>
#include <memory>

#include <d3d11.h>

namespace renderer {
	class texture2d {
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;
	};

	// TODO: Texture atlas and UV mapping to batch textures
	class texture_atlas {
	private:
		std::unique_ptr<texture2d> texture;
		std::map<size_t, glm::vec4> textures;

	public:
		// size_t add_texture(texture2d* texture);
	};
}// namespace renderer

#endif