#pragma once

#include "engine/internal_libs.hpp"
#include "engine/graphics/interface/window.hpp"
#include "engine/graphics/vulkan/vk_instance.hpp"
#include "engine/graphics/vulkan/vk_physical_device.hpp"
#include "engine/graphics/vulkan/vk_logical_device.hpp"
#include "engine/graphics/vulkan/vk_swapchain.hpp"

#include <vulkan/vulkan.hpp>
#include <string>


namespace oe::graphics {

	class VKWindow : public Window {
	public:
		const VKInstance* m_instance;
		vk::SurfaceKHR m_surface;
		bool m_debugging;

		PhysicalDevice* m_physical_device;
		LogicalDevice* m_logical_device;
		SwapChain* m_swapchain;

	public:
		VKWindow(const VKInstance* instance, const WindowInfo& window_config);
		~VKWindow();

		void glfw();

		// Inherited via Window
		virtual void update() override;
		virtual void clear(const glm::vec4& color = oe::colors::clear_color) override;
		virtual void viewport() override;
		virtual void swapInterval(uint8_t frames) override;

		virtual Renderer* createRenderer(const RendererInfo& renderer_info) const override;
		virtual Shader* createShader(const ShaderInfo& shader_info) const override;
		virtual Texture* createTexture(const TextureInfo& texture_info) const override;
		virtual FrameBuffer* createFrameBuffer(const FrameBufferInfo& framebuffer_info) const override;
		
		virtual void destroyRenderer(Renderer* renderer) const override;
		virtual void destroyShader(Shader* shader) const override;
		virtual void destroyTexture(Texture* texture) const override;
		virtual void destroyFrameBuffer(FrameBuffer* framebuffer) const override;

		// Inherited via Instance
		virtual std::string getAPI() const override;
		virtual std::string getAPIVersion() const override;
		virtual std::string getGPU() const override;
		virtual std::string getGPUVendor() const override;
	};

}