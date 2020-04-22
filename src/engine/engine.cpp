#include "engine.hpp"

#include "graphics/opengl/gl_instance.hpp"
#include "graphics/vulkan/vk_instance.hpp"

#include <GLFW/glfw3.h>



static void glfw_error_func(int code, const char* desc) {
	spdlog::error("GLFW ({}): {}", code, desc);
}

namespace oe {

	Engine* Engine::singleton = nullptr;

	Engine::Engine() {
		// init to start the timer and seed the randomizer
		utils::Random::getSingleton().seed(utils::Clock::getSingleton().getMicroseconds());
	}

	void Engine::init(EngineInfo _engine_info) {
		spdlog::set_pattern("%^[%T] [%l]:%$ %v");
		spdlog::set_level(spdlog::level::level_enum::trace);

		engine_info = _engine_info;

		glfwSetErrorCallback(glfw_error_func);
		if (!glfwInit()) oe_error_terminate("Failed to initialize GLFW!");
		
		if (engine_info.audio) {
			audio::Audio::init();
		}
		if (engine_info.networking) {
			networking::enet::initEnet();
		}

		srand(oe::utils::Clock::getSingleton().getMicroseconds());
	}

	void Engine::deinit() {
		if (engine_info.audio) {
			audio::Audio::deinit();
		}
		if (engine_info.networking) {
			networking::enet::deinitEnet();
		}
	}

	void Engine::terminate() {
#ifdef _DEBUG
		assert(0);
#else
#endif // _DEBUG
	}

	void Engine::__error(std::string error_msg, int line, std::string file) {
		spdlog::error("error: {}\nline: {}\nfile: {}", error_msg, line, file);
		oe::Engine::terminate();
	}

	graphics::Instance* Engine::createInstance(const InstanceInfo& instance_config) {
		if (engine_info.api == oe::graphics_api::OpenGL) {
			return new oe::graphics::GLInstance(instance_config);
		}
		else {
#ifdef BUILD_VULKAN
			return new oe::graphics::VKInstance(instance_config);
#else
			return nullptr;
#endif
		}
	}

	void Engine::destroyInstance(graphics::Instance* instance) {
		delete instance;
	}

	void Engine::multipass(graphics::FrameBuffer& fb_0, graphics::FrameBuffer& fb_1, const graphics::Renderer& renderer, size_t count) {
		for (int i = 0; i < count; i++) {
			fb_1.bind();
			fb_0.getTexture()->bind();
			renderer.render(1);
			fb_1.unbind();

			fb_0.bind();
			fb_1.getTexture()->bind();
			renderer.render(1);
			fb_0.unbind();
		}
	}

}