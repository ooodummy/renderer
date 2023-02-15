#ifndef RENDERER_D3D11_TEXTURE2D_HPP
#define RENDERER_D3D11_TEXTURE2D_HPP

#include <d3d11.h>

namespace renderer {
	class d3d11_texture2d {
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;
	};
}// namespace renderer

#endif