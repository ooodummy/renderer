#include <carbon/carbon.hpp>
#include <renderer/core.hpp>

#include <dwmapi.h>
#include <thread>
#include <windowsx.h>

#include <glm/gtx/rotate_vector.hpp>
#include <fmt/core.h>

renderer::sync_manager updated_draw;
renderer::sync_manager updated_buf;

bool update_size = false;
bool close_requested = false;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
			close_requested = true;
            return 0;
        case WM_SIZE:
            carbon::application->set_size({LOWORD(lParam), HIWORD(lParam)});
            update_size = true;
            break;
        default:
            break;
    }

	if (carbon::impl_win32_winproc_handler(hWnd, msg, wParam, lParam))
		return 1;

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void draw_test_primitives(renderer::buffer* buf) {
	static renderer::timer rainbow_timer;
	auto elapsed_ms = rainbow_timer.get_elapsed_duration().count();

	if (elapsed_ms > 5000)
		rainbow_timer.reset();

	renderer::color_rgba rainbow = renderer::color_hsva(0.0f).ease(renderer::color_hsva(359.99f),
																		 static_cast<float>(elapsed_ms) / 5000.0f);
	rainbow.a = 75;

	static std::vector<glm::vec2> points = {
		{400.0f, 500.0f},
		{700.0f, 500.0f},
		{600.0f, 350.0f},
		{700.0f, 300.0f},
		{500.0f, 200.0f},
		{500.0f, 600.0f},
		{600.0f, 600.0f}};

	// TODO: Polylines are broken
	static auto polyline = renderer::polyline_shape(points, rainbow, 20.0f, renderer::joint_miter);
	polyline.set_color(rainbow);

	auto mouse = carbon::get_mouse_pos();

	const glm::vec4 scissor_bounds = {
		static_cast<float>(mouse.x) - 50.0f,
		static_cast<float>(mouse.y) - 50.0f,
		100.0f,
		100.0f};

	buf->push_key(COLOR_RED);

	{
		glm::vec2 cube_offset = {50.0f, 50.0f};
		const auto cube_size = 20.0f;
		const auto cube_half = cube_size / 2.0f;
		const auto cube_double = cube_size * 2.0f;
		const auto cube_space = cube_size * 2.5f;

		buf->draw_rect_filled({cube_offset, cube_size, cube_size}, COLOR_RED);

		buf->push_scissor(scissor_bounds);
		buf->draw_rect_filled({cube_offset + glm::vec2(cube_double, cube_half), cube_size, cube_size}, COLOR_BLUE);
		buf->pop_scissor();

		buf->draw_rect_filled({cube_offset + glm::vec2(cube_half, cube_double), cube_size, cube_size}, COLOR_GREEN);
		buf->draw_rect_filled({cube_offset + glm::vec2(cube_space, cube_space), cube_size, cube_size}, COLOR_YELLOW);
	}

	buf->pop_key();

	// TODO: Fix circle polyline
	//buf->draw_circle({300.0f, 100.0f}, 100.0f, {255, 255, 255, 125}, 10.0f);
	//buf->draw_circle_filled({300.0f, 100.0f}, 50.0f, {255, 255, 0, 155});

	buf->push_scissor(scissor_bounds, true);

	buf->add_shape(polyline);

	buf->pop_scissor();

	//const std::string test_string = "Hello World!";
	//buf->draw_text({250.0f, 250.0f}, test_string, segoe);
	//buf->draw_rect(dx11->get_text_bounds({250.0f, 250.0f}, test_string, segoe), COLOR_YELLOW);

	buf->draw_rect(scissor_bounds, COLOR_WHITE);

	//buf->draw_rect({1.0f, 1.0f, 3.0f, 3.0f}, COLOR_BLUE);
	//buf->draw_rect_filled({1.0f, 1.0f, 2.0f, 2.0f}, COLOR_RED);

	// Testing arc performance
	buf->draw_rect_rounded({100.0f, 200.0f, 200.0f, 150.0f}, 0.3f, COLOR_YELLOW, 15.0f);
	buf->draw_rect_rounded_filled({350.0f, 200.0f, 200.0f, 150.0f}, 0.3f, COLOR_GREEN);
	buf->draw_arc({700.0f, 275.0f}, 0.0f, M_PI, 100.0f, COLOR_BLUE, 0.0f, 32, true);
	buf->draw_circle_filled({950.0f, 300.0f}, 100.0f, COLOR_RED, 64);

	//D3DX11CreateShaderResourceViewFromFile(dx11->, L"braynzar.jpg",NULL, NULL, &CubesTexture, NULL );

	/*static std::vector<glm::vec2> points = {
		{100.0f, 500.0f},
		{100.0f, 100.0f},
		{500.0f, 100.0f}
	};

	static auto polyline = renderer::polyline_shape(points, {255, 255, 255, 150}, 50.0f, renderer::joint_miter);
	buf->add_shape(polyline);*/
}

void draw_test_bezier(renderer::buffer* buf) {
	static renderer::timer timer;
	if (timer.get_elapsed_duration() > std::chrono::seconds(1))
		timer.reset();

	const auto t = renderer::ease(0.0f, 1.0f, std::chrono::duration<float>(timer.get_elapsed_duration()).count(),
								  renderer::ease_type::in_sine);

	static renderer::bezier_curve<3> bezier;
	bezier[0] = {200.0f, 300.0f};
	bezier[1] = carbon::get_mouse_pos();
	bezier[2] = {400.0f, 400.0f};
	bezier[3] = {500.0f, 200.0f};

	buf->draw_bezier_curve(bezier, COLOR_RED, 5.0f, renderer::cap_butt, 32);

	const auto point = bezier.position_at(t);
	const auto tangent = bezier.tangent_at(t);
	const auto angle = atan2f(tangent.y, tangent.x);

	buf->draw_line(point, glm::rotate(glm::vec2(60.0f, 0.0f), angle) + point, COLOR_GREEN);
	buf->draw_circle_filled(point, 5.0f, COLOR_BLACK);

	glm::vec2 prev{};
	for (size_t i = 0; i < bezier.size(); i++) {
		const auto& control_point = bezier[i];

		buf->draw_circle_filled(control_point, 5.0f, {255, 255, 255, 100});

		if (prev != glm::vec2{})
			buf->draw_line(prev, control_point, COLOR_WHITE);

		prev = control_point;
	}
}

void draw_test_flex(renderer::buffer* buf) {
	static bool init = false;
	static auto flex_container = std::make_unique<carbon::flex_container>();

	if (!init) {
		flex_container->set_pos({50.0f, 50.0f});
		//flex_container->set_justify_content(carbon::justify_center);
		/*const auto item1 = flex_container->add_child<carbon::flex_item>();
		item1->set_basis(25.0f, carbon::unit_pixel);
		const auto item2 = flex_container->add_child<carbon::flex_item>();
		item2->set_basis(50.0f, carbon::unit_pixel);
		const auto item3 = flex_container->add_child<carbon::flex_item>();
		item3->set_basis(50.0f, carbon::unit_pixel);*/
		//const auto item4 = flex_container->add_child<carbon::flex_item>();
		//item4->set_basis(125.0f, carbon::unit_pixel);

		const auto container1 = flex_container->add_child<carbon::flex_container>();
		container1->set_flex(1.0f);
		container1->set_flow(carbon::column);
		container1->set_max_width(300.0f);
		const auto item3 = flex_container->add_child<carbon::flex_item>();
		item3->set_flex(1.0f);
		const auto container11 = container1->add_child<carbon::flex_container>();
		container11->set_flex(1.0);
		container11->set_min_width(50.0f);
		container11->set_max_width(100.0f);
		const auto item111 = container11->add_child<carbon::flex_item>();
		item111->set_flex(1.0f);
		const auto item112 = container11->add_child<carbon::flex_item>();
		item112->set_flex(1.0f);
		const auto item113 = container11->add_child<carbon::flex_item>();
		item113->set_flex(1.0f);
		const auto container12 = container1->add_child<carbon::flex_container>();
		container12->set_flex(1.0f);
		container12->set_flow(carbon::column);
		const auto item121 = container12->add_child<carbon::flex_item>();
		item121->set_flex(1.0f);
		const auto item122 = container12->add_child<carbon::flex_item>();
		item122->set_flex(1.0f);
		container12->set_max_width(100.0f);
		auto container2 = container1->add_child<carbon::flex_container>();
		container2->set_flex(1.0f);
		container2->set_min_width(150.0f);

		init = true;
	}

	flex_container->set_size(carbon::get_mouse_pos() - flex_container->get_pos());
	flex_container->compute();
	flex_container->draw();
}

void draw_test_text(renderer::buffer* buf) {
	buf->draw_text<std::u32string>({100.0f, 100.0f}, U"😎 \u26F0 👻 😈 🙊 🚀", carbon::segoe_font);

	const std::string text_string = "Hello World!";
	const auto text_size = carbon::dx11->get_text_size(text_string, carbon::segoe_font);

	buf->draw_text<std::string>({250.0f, 250.0f}, text_string, carbon::segoe_font, COLOR_WHITE);
	buf->draw_rect({250.0f, 250.0f, text_size}, COLOR_RED);
}

void draw_test_ui(renderer::buffer* buf) {
	static auto menu = std::make_unique<carbon::window>();
	static bool init = false;
	static carbon::label<std::string>* label = nullptr;

	if (!init && carbon::is_key_down('F')) {
		auto groupbox = std::make_unique<carbon::groupbox<std::string>>("Aimbot");
		label = groupbox->body->add_child<carbon::label<std::string>>("Aimbot enabled", carbon::segoe_font,
																	  COLOR_WHITE);
		groupbox->body->add_child<carbon::label<std::string>>("Visibility check", carbon::segoe_font, COLOR_WHITE);
		menu->content->add_child(std::move(groupbox));

		groupbox = std::make_unique<carbon::groupbox<std::string>>("Accuracy");
		label = groupbox->body->add_child<carbon::label<std::string>>("Remove sway", carbon::segoe_font, COLOR_WHITE);
		groupbox->body->add_child<carbon::label<std::string>>("Remove recoil", carbon::segoe_font, COLOR_WHITE);
		menu->content->add_child(std::move(groupbox));

		menu->set_pos({300.0f, 300.0f});
		menu->set_size({580.0f, 500.0f});
		init = true;
	}

	/*static int i = 0;
	i++;
	std::string test = std::to_string(i);
	label->set_label(test);*/

	//menu->set_size(carbon::get_mouse_pos() - menu->get_pos());

	menu->input();
	menu->compute();
	menu->draw();
}

void draw_input_data(renderer::buffer* buf) {
	static auto overlay_container = std::make_unique<carbon::flex_container>();
	static bool init = false;

	static carbon::label<std::string>* label = nullptr;

	if (!init) {
		init = true;

		// Create labels
	}

	// Update labels

	const auto mouse = carbon::get_mouse_pos();
	buf->draw_text<std::string>({25.0f, 25.0f}, fmt::format("Mouse position: ({}, {})", mouse.x, mouse.y),
								  carbon::segoe_font);
	buf->draw_text<std::string>({25.0f, 50.0f}, fmt::format("Mouse state: {} {}", carbon::is_key_pressed(VK_LBUTTON), carbon::is_key_down(VK_LBUTTON)), carbon::segoe_font);
	buf->draw_text<std::string>({25.0f, 75.0f}, fmt::format("Batches: {}", carbon::benchmark.draw_calls,
															 carbon::is_key_down(VK_LBUTTON)), carbon::segoe_font);
	buf->draw_text<std::string>({25.0f, 100.0f}, fmt::format("Flex compute: {}", carbon::benchmark.flex_compute_calls,
															  carbon::is_key_down(VK_LBUTTON)), carbon::segoe_font);
}

#include "force_engine/forces/collide.hpp"
#include "force_engine/forces/radial.hpp"
#include "force_engine/quadtree.hpp"
#include "force_engine/simulation.hpp"

void draw_force_simulation(renderer::buffer* buf) {
	static glm::vec2 simulation_offset = { carbon::application->get_size().x / 2.0f, carbon::application->get_size().y / 2.0f };
	static auto simulation = std::make_unique<engine::simulation>();
	static engine::force_link* simulation_links = nullptr;
	static engine::force_collide* simulation_colliders = nullptr;
	static engine::force_radial* simulation_radial = nullptr;
	//static engine::node* mouse_collider = nullptr;
	static bool test = false;

	if (!test) {
		test = true;

		const auto a = simulation->add_node();
		const auto b = simulation->add_node();
		const auto c = simulation->add_node();
		const auto e = simulation->add_node();
		const auto f = simulation->add_node();
		const auto g = simulation->add_node();
		const auto h = simulation->add_node();

		for (size_t i = 0; i < 25; i++) {
			simulation->add_node();
		}

		simulation->initialize_nodes();

		std::vector<engine::link> links = {
			{a, b},
			{b, c},
			{c, a},
			{e, c},
			{f, e},
			{g, h}
		};

		//simulation->add_force<engine::force_center>("center", simulation->get_nodes());
		simulation_links = simulation->add_force<engine::force_link>("link", links);
		simulation_colliders = simulation->add_force<engine::force_collide>("collide", simulation->get_nodes());
		simulation_radial = simulation->add_force<engine::force_radial>("radial", simulation->get_nodes(), 200.0f, glm::vec2{});

		simulation->initialize_forces();
	}

	static renderer::timer timer;
	if (timer.get_elapsed_duration() >= std::chrono::milliseconds(5)) {
		timer.reset();

		simulation->step();
	}

	auto draw_quad = [&buf](engine::quadtree& tree, auto self_ref) -> void { // NOLINT(misc-no-recursion)
		for (auto& quad : tree.get_children()) {
			if (quad) {
				const auto bounds = quad->get_bounds();
				buf->draw_rect({bounds.x + simulation_offset.x, bounds.y + simulation_offset.y, bounds.z, bounds.w}, { 255, 255, 255, 75 }, 1.0f);
				self_ref(*quad, self_ref);
			}
		}
	};

	engine::quadtree quadtree(simulation->get_nodes());
	draw_quad(quadtree, draw_quad);

	const auto mouse = carbon::get_mouse_pos();
	std::vector<engine::node*> hovered_nodes;
	engine::node* closest = nullptr;
	float closest_distance = std::numeric_limits<float>::infinity();

	for (auto& node : simulation->get_nodes()) {
		const auto draw_position = node->position + simulation_offset;

		const  auto distance = glm::distance(mouse, draw_position);
		if (distance < 30.0f) {
			if (distance < closest_distance) {
				closest_distance = distance;
				closest = node;
			}

			hovered_nodes.push_back(node);
		}
	}

	static engine::node* held = nullptr;
	if (GetAsyncKeyState(VK_LBUTTON)) {
		if (held) {
			closest = held;
			held->position = mouse - simulation_offset;
		}
		else if (closest) {
			held = closest;
		}

		if (closest) {
			closest->position = mouse - simulation_offset;
		}
	}
	else {
		held = nullptr;
	}

	for (auto& node : hovered_nodes) {
		const auto draw_position = node->position + simulation_offset;

		buf->draw_line(mouse, draw_position, COLOR_RED);
	}

	for (auto& link : simulation_links->get_links()) {
		const auto a = link.source->position + simulation_offset;
		const auto b = link.target->position + simulation_offset;

		buf->draw_line(a, b, COLOR_BLACK);
	}

	for (auto& node : simulation->get_nodes()) {
		const auto draw_position = node->position + simulation_offset;

		buf->draw_circle_filled(draw_position, node->radius - 5, COLOR_BLACK, 12);
		buf->draw_circle_filled(draw_position, node->radius - 8.0f, COLOR_WHITE, 12);
	}

	buf->draw_circle(mouse, 30.0f, COLOR_RED);
}

void draw_thread() {
    const auto id = carbon::dx11->register_buffer();

    while (!close_requested) {
		updated_draw.wait();

		carbon::buf = carbon::dx11->get_working_buffer(id);

		carbon::begin();
		carbon::benchmark.flex_compute_calls = 0;

		//draw_test_primitives(carbon::buf);
		//draw_test_bezier(carbon::buf);
		//draw_test_flex(carbon::buf);
		//draw_force_simulation(carbon::buf);
		//draw_test_text(carbon::buf);
		draw_test_ui(carbon::buf);
		draw_input_data(carbon::buf);

		carbon::end();

		carbon::dx11->swap_buffers(id);
        updated_buf.notify();
    }
}

// TODO: Mutex for textures
int main() {
    carbon::application = std::make_unique<renderer::win32_window>();
	carbon::application->set_title("D3D11 Renderer");
	carbon::application->set_size({1920, 1080});

    // Center window position
    {
        RECT client;
        if (GetClientRect(GetDesktopWindow(), &client)) {
            const auto size = carbon::application->get_size();
			carbon::application->set_pos({client.right / 2 - size.x / 2, client.bottom / 2 - size.y / 2});
        }
    }

	carbon::application->set_proc(WndProc);

    if (!carbon::application->create()) {
        MessageBoxA(nullptr, "Failed to create window.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    /*{
        auto attribute = DWMWCP_DONOTROUND;
        DwmSetWindowAttribute(window->get_hwnd(), DWMWA_WINDOW_CORNER_PREFERENCE, &attribute, sizeof(attribute));
    }*/

    carbon::dx11 = std::make_unique<renderer::d3d11_renderer>(carbon::application.get());

    if (!carbon::dx11->init()) {
        MessageBoxA(nullptr, "Failed to initialize renderer.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

	carbon::dx11->set_vsync(false);
	carbon::dx11->set_clear_color({88, 88, 88});//({88, 122, 202});

    carbon::segoe_font = carbon::dx11->register_font("Segoe UI Emoji", 10, FW_THIN, true);

	carbon::init();

    std::thread draw(draw_thread);

	carbon::application->set_visibility(true);

	MSG msg{};
    while (!close_requested && msg.message != WM_QUIT) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		if (msg.message == WM_NULL && !IsWindow(carbon::application->get_hwnd())) {
			close_requested = true;
			break;
		}

        if (update_size) {
			carbon::dx11->resize();
			carbon::dx11->reset();

            update_size = false;
        }

		carbon::dx11->draw();
		carbon::benchmark.draw_calls = carbon::dx11->total_batches;

		updated_draw.notify();
        updated_buf.wait();
    }

    draw.join();

	carbon::dx11->release();
	carbon::application->destroy();

    return 0;
}