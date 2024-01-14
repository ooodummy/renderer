#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "color.hpp"
#include "font.hpp"
#include "texture.hpp"

#include <glm/glm.hpp>
#include <shared_mutex>
#include <stack>

namespace renderer {
	class buffer;
	struct shared_data;

	struct buffer_node {
		std::unique_ptr<buffer> active;
		std::unique_ptr<buffer> working;

		std::vector<std::pair<size_t, size_t>> child_buffers;
		bool free = false;
		size_t parent = std::numeric_limits<size_t>::max();
	};

	// TODO: Most of the abstraction has been removed since I just want a functional D3D11 renderer currently and I
	//  don't need a Vulkan/OpenGL renderer right now but would like to later add that work and keep this up as a
	//  passion project.
	class d3d11_renderer : public i_device_notify {
	public:
		explicit d3d11_renderer(std::shared_ptr<win32_window> window);

		explicit d3d11_renderer(IDXGISwapChain* swap_chain);

		bool initialize();
		bool release();

		void on_window_moved();
		void on_display_change();
		void on_window_size_change(glm::i16vec2 size);

	private:
		void clear();

		void resize_buffers();
		void draw_batches();

		void create_device_dependent_resources();
		void create_window_size_dependent_resources();

		void on_device_lost() override;
		void on_device_restored() override;

	public:
		void render();

		size_t register_buffer(size_t priority = 0, size_t vertices_reserve_size = 0, size_t indices_reserve_size = 0, size_t batches_reserve_size = 0);
		size_t register_child_buffer(size_t parent, size_t priority = 0, size_t vertices_reserve_size = 0, size_t indices_reserve_size = 0, size_t batches_reserve_size = 0);

		void update_buffer_priority(size_t id, size_t priority = 0);
		void update_child_buffer_priority(size_t id, size_t priority = 0);

		void remove_buffer(size_t id);

		buffer* get_working_buffer(size_t id);

		void swap_buffers(size_t id);

		void create_atlases();
		void destroy_atlases();

		void set_clear_color(const color_rgba& color);

		glm::vec2 get_render_target_size();
		[[nodiscard]] const std::unique_ptr<shared_data>& get_shared_data() const {
			return shared_data_;
		}

	private:
		std::shared_mutex buffer_list_mutex_;

        // TODO: Store buffers in a map so if we remove a buffer we can just complelety erase it instead of just freeing
        //  it for later
		std::vector<buffer_node> buffers_;
        std::vector<std::pair<size_t, size_t>> priorities_;
        std::vector<size_t> free_buffers_;

		std::unique_ptr<renderer_context> context_;

		bool msaa_enabled_;

		int16_t target_sample_count_;
		int16_t sample_count_;

		std::unique_ptr<shared_data> shared_data_;

		ComPtr<ID3D11Texture2D> msaa_render_target_;
		ComPtr<ID3D11RenderTargetView> msaa_render_target_view_;
		ComPtr<ID3D11DepthStencilView> msaa_depth_stencil_view_;

		glm::vec4 clear_color_;

		// Backup render states
		struct backup_d3d11_state {
			UINT scissor_rects_count;
			UINT viewports_count;
			D3D11_RECT scissor_rects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			ID3D11RasterizerState* rasterizer_state;
			ID3D11BlendState* blend_state;
			FLOAT blend_factor[4];
			UINT sample_mask;
			UINT stencil_ref;
			ID3D11DepthStencilState* depth_stencil_state;
			ID3D11ShaderResourceView* pixel_shader_shader_resource;
			ID3D11SamplerState* pixel_shader_sampler;
			ID3D11PixelShader* pixel_shader;
			ID3D11VertexShader* vertex_shader;
			ID3D11GeometryShader* geometry_shader;
			UINT pixel_shader_instances_count;
			UINT vertex_shader_instances_count;
			UINT geometry_shader_instances_count;
			ID3D11ClassInstance* pixel_shader_instances[256];
			ID3D11ClassInstance* vertex_shader_instances[256];
			ID3D11ClassInstance* geometry_shader_instances[256];
			D3D11_PRIMITIVE_TOPOLOGY primitive_topology;
			ID3D11Buffer* index_buffer;
			ID3D11Buffer* vertex_buffer;
			ID3D11Buffer* pixel_shader_constant_buffer;
			ID3D11Buffer* vertex_shader_constant_buffer;
			UINT index_buffer_offset;
			UINT vertex_buffer_stride;
			UINT vertex_buffer_offset;
			DXGI_FORMAT index_buffer_format;
			ID3D11InputLayout* input_layout;
		};

		backup_d3d11_state state_;

		void setup_states();

		void backup_states();

		void restore_states();
	};
}// namespace renderer

#endif