#ifndef _RENDERER_D3D11_TEXTURE2D_HPP_
#define _RENDERER_D3D11_TEXTURE2D_HPP_

#include <d3d11.h>

namespace renderer {
	class texture2d {
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* srv;
	};
}// namespace renderer

#endif