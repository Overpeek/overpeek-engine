#include "gui_manager.hpp"

#include "engine/graphics/interface/window.hpp"
#include "engine/graphics/interface/renderer.hpp"
#include "engine/graphics/font.hpp"
#include "widgets/form.hpp"
#include "engine/include.hpp"



namespace oe::gui {

	constexpr int border = 5;

	

	GUI::GUI(const GUIBase& base)
		: m_guibase(base)
	{
		// renderer
		oe::RendererInfo renderer_info = {};
		renderer_info.arrayRenderType = oe::types::dynamic_type;
		renderer_info.indexRenderType = oe::types::static_type;
		renderer_info.max_primitive_count = 10000;
		renderer_info.staticVBOBuffer_data = nullptr;
		m_renderer = new oe::graphics::Renderer(renderer_info);
		m_late_renderer = new oe::graphics::Renderer(renderer_info);

		// shader
		m_shader = oe::assets::DefaultShader();

		FormInfo form_info = {};
		form_info.size = m_guibase.get_size_func() - glm::vec2(2 * border);
		form_info.offset_position = { border, border };
		m_main_frame = new oe::gui::Form(form_info);
		m_offset = { 0, 0 };
		m_old_window_size = { 0, 0 };

		m_main_frame->managerAssigned(this);

		// initial resize
		short_resize();
		
		// event listeners
		m_guibase.resize_connect_func(this);
	}

	GUI::GUI(const oe::graphics::Window& window)
		: GUI(GUIBase::GUIBase(
			std::bind(&oe::graphics::IWindow::getSize, window.get()), 
			std::bind((void(oe::graphics::IWindow::*)(GUI *const &))&oe::graphics::IWindow::connect_listener<oe::ResizeEvent, &GUI::on_resize>, window.get(), std::placeholders::_1),
			std::bind((void(oe::graphics::IWindow::*)(GUI *const &))&oe::graphics::IWindow::disconnect_listener<oe::ResizeEvent, &GUI::on_resize>, window.get(), std::placeholders::_1),
			window
		))
	{
	}

	GUI::~GUI() {
		delete m_main_frame;
		delete m_renderer;
		delete m_late_renderer;
		
		// event listeners
		m_guibase.resize_disconnect_func(this);
	}

	void GUI::offset(const glm::vec2& offset) {
		m_offset = offset;

		glm::mat4 ml_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(offset, 0.0f));
		m_shader.bind();
		m_shader.setModelMatrix(ml_matrix);
	}

	void GUI::render() {
		auto& engine = oe::Engine::getSingleton();
		auto old_depth = engine.getDepth();
		engine.depth(oe::depth_functions::always);

		render_empty();

		m_shader.bind();
		m_renderer->render();
		m_late_renderer->render();
		
		engine.depth(old_depth);
	}

	void GUI::render_empty()
	{
		short_resize();
		float z = 0.0f;
		GUIRenderEvent event;
		event.z = &z;
		dispatcher.trigger(event);
	}

	void GUI::short_resize() {
		oe::ResizeEvent event;
		event.framebuffer_size = m_guibase.get_size_func();
		event.framebuffer_size_old = event.framebuffer_size;
		on_resize(event);
	}

	void GUI::on_resize(const oe::ResizeEvent& event) {
		m_main_frame->size = static_cast<glm::vec2>(event.framebuffer_size) - glm::vec2(2 * border);
		m_main_frame->offset_position = { border, border };

		/*
		    0                  0
		  0 +------------------+ w
		    |                  |
		    |                  |
		    |                  |
		  0 +------------------+ w
		    h                  h
		*/

		if (m_old_window_size == event.framebuffer_size) return;
		glm::mat4 pr_matrix = glm::ortho(0.0f, (float)event.framebuffer_size.x, (float)event.framebuffer_size.y, 0.0f, -100000.0f, 10.0f);
		m_shader.bind();
		m_shader.useTexture(true);
		m_shader.setProjectionMatrix(pr_matrix);
		m_old_window_size = event.framebuffer_size;
	}

	void GUI::addSubWidget(Widget* widget)
	{
		m_main_frame->addSubWidget(widget);
	}

	void GUI::removeSubWidget(Widget* widget)
	{
		m_main_frame->removeSubWidget(widget);
	}

}