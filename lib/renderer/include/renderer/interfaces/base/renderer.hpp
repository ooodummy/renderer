#ifndef _RENDERER_INTERFACES_BASE_RENDERER_HPP_
#define _RENDERER_INTERFACES_BASE_RENDERER_HPP_

namespace renderer {
	class buffer;

	struct buffer_node {
		std::unique_ptr<buffer> active;
		std::unique_ptr<buffer> working;
	};


	class base_renderer {

	};
}

#endif