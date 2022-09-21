#ifndef _RENDERER_IMAGE_NINE_SLICE_HPP_
#define _RENDERER_IMAGE_NINE_SLICE_HPP_

#include "../d3d11/texture2d.hpp"
#include <cstdint>

// TODO: Implement 9 patch/slice scheme similar to android
// https://github.com/NXPmicro/gtec-demo-framework/blob/master/Doc/FslSimpleUI.md#n-patch
// https://developer.android.com/studio/write/draw9patch
namespace renderer {
	enum class patch_scaling : uint8_t {
		no_scaling,
		scaling
	};

	struct patch {
		patch_scaling scaling;
		float width;
	};

	template <size_t N>
	class npatch {
		patch patches[N];
		texture2d unscaled_texture;
	};
}

#endif