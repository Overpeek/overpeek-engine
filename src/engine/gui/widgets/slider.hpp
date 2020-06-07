#pragma once

#include "widget.hpp"



namespace oe::graphics { class Quad; class TextLabel; }
namespace oe::gui {

	typedef std::function<void(float)> slider_callback;

	struct SliderInfo {
		slider_callback callback                   = nullptr;
		float min_value                            = -1.0f;
		float max_value                            = 1.0f;
		float initial_value                        = 0.0f;
		bool draw_value                            = false;
		glm::ivec2 slider_size                     = { 50, 50 };
		glm::ivec2 knob_size                       = { 50, 50 };
		glm::vec4 knob_color                       = oe::colors::grey;
		glm::vec4 slider_lcolor                    = oe::colors::dark_grey;
		glm::vec4 slider_rcolor                    = oe::colors::darker_grey;
		const oe::graphics::Sprite* knob_sprite    = nullptr; // must be set
		const oe::graphics::Sprite* slider_sprite  = nullptr; // must be set
		glm::vec2 offset_position                  = { 0, 0 };
		glm::vec2 align_parent                     = oe::alignments::center_center;
		glm::vec2 align_render                     = oe::alignments::center_center;
	};

	struct GUIRenderEvent;
	class GUI;
	class Slider : public Widget {
	private:
		oe::graphics::TextLabel* value_label;
		oe::graphics::Quad* label_quad;
		oe::graphics::Quad* quad_knob;
		oe::graphics::Quad* quad_lslider;
		oe::graphics::Quad* quad_rslider;

	private:
		bool m_dragging;

	public:
		SliderInfo slider_info;

	public:
		Slider(GUI* gui_manager, const SliderInfo& slider_info);
		~Slider();

		// events
		void on_render(const GUIRenderEvent& event);
		void on_cursor(const CursorPosEvent& event);
		void on_button(const MouseButtonEvent& event);
	};

}