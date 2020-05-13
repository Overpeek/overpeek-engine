#include "text_input.hpp"

#include "engine/graphics/textLabel.hpp"
#include "engine/include.hpp"
#include "engine/graphics/interface/window.hpp"
#include "engine/gui/gui_manager.hpp"

bool insertchars(std::string *obj, int i, char *chars, int n) {
	obj->insert(i, chars, n);
	return true;
}

#define STB_TEXTEDIT_STRING						std::string
#define STB_TEXTEDIT_CHARTYPE					char

#define STB_TEXTEDIT_STRINGLEN(obj)				obj->size()
#define STB_TEXTEDIT_LAYOUTROW(r,obj,n)			NULL
#define STB_TEXTEDIT_GETWIDTH(obj,n,i)			NULL
#define STB_TEXTEDIT_GETCHAR(obj,i)				obj->at(i)
#define STB_TEXTEDIT_NEWLINE					'\n'
#define STB_TEXTEDIT_DELETECHARS(obj,i,n)		obj->erase(i, n)
#define STB_TEXTEDIT_INSERTCHARS(obj,i,c,n)		insertchars(obj, i, c, n)
#define STB_TEXTEDIT_KEYTOTEXT(k)				k
#define STB_TEXTEDIT_K_UNDO						300
#define STB_TEXTEDIT_K_REDO						301
#define STB_TEXTEDIT_K_LEFT						302
#define STB_TEXTEDIT_K_RIGHT					303
#define STB_TEXTEDIT_K_DOWN						304
#define STB_TEXTEDIT_K_UP						305
#define STB_TEXTEDIT_K_DELETE					306
#define STB_TEXTEDIT_K_BACKSPACE				307
#define STB_TEXTEDIT_K_TEXTSTART				308	// ctrl home
#define STB_TEXTEDIT_K_TEXTEND					309 // ctrl end
#define STB_TEXTEDIT_K_LINESTART				310	// home
#define STB_TEXTEDIT_K_LINEEND					311 // end
#define STB_TEXTEDIT_K_WORDLEFT					312 // ctrl left
#define STB_TEXTEDIT_K_WORDRIGHT				313 // ctrl right
#define STB_TEXTEDIT_K_SHIFT					0x10000

#define STB_TEXTEDIT_IMPLEMENTATION
#include <stb_textedit.h>



namespace oe::gui {

	float timer_key_pressed = 0;
	void resetTimer() {
		auto& clock = oe::utils::Clock::getSingleton();
		timer_key_pressed = clock.getSessionMillisecond() + 1000.0f;
	}
	
	TextInput::TextInput(GUI* gui_manager, const TextInputInfo& _text_input_info)
		: Widget(gui_manager, _text_input_info.size, _text_input_info.align_parent, _text_input_info.align_render, _text_input_info.offset_position)
		, text_input_info(_text_input_info)
		, m_selected(false)
		, m_filtering(_text_input_info.filter != filter_none)
	{
		m_state = new STB_TexteditState();
		static_cast<STB_TexteditState*>(m_state)->cursor = _text_input_info.text.size();

		quad = m_gui_manager->getRenderer()->createQuad();
		text_quad = m_gui_manager->getLateRenderer()->createQuad();
		label = new oe::graphics::TextLabel();
	}

	TextInput::~TextInput() {
		m_gui_manager->getRenderer()->destroyQuad(quad);
		m_gui_manager->getLateRenderer()->destroyQuad(text_quad);
		delete m_state;
		delete label;
	}

	void TextInput::render(float& z, oe::graphics::Renderer* renderer) {
		// bounding box
		quad->setPosition(render_position - text_input_info.padding);
		quad->setZ(z);
		quad->setSize(size + text_input_info.padding * 2.0f);
		quad->setSprite(text_input_info.sprite);
		quad->setColor(text_input_info.color);
		quad->update();

		// vertical bar
		std::string bar = "";
		auto& clock = oe::utils::Clock::getSingleton();
		float time = clock.getSessionMillisecond();
		if (m_selected && (timer_key_pressed > clock.getSessionMillisecond() || (int)floor(time) % 2000 > 1000))
		{
			bar = "|";
		}

		// text
		z += 1.0f;
		glm::vec2 text_size = glm::vec2(text_input_info.font_size);
		// label->generate(fmt::format("<#000000>{}", text_input_info.text), m_gui_manager->getWindow(), oe::colors::pink);
		label->generate(fmt::format("<#000000>{}{}", text_input_info.text, bar), m_gui_manager->getWindow());
		text_quad->setPosition(render_position + oe::alignmentOffset(size, text_input_info.align_text) - oe::alignmentOffset(text_size * label->getSize(), text_input_info.align_text));
		text_quad->setZ(z);
		text_quad->setSize(text_size * label->getSize());
		text_quad->setSprite(label->getSprite());
		text_quad->setColor(oe::colors::white);
		text_quad->update();
	}

	void TextInput::text(uint32_t codepoint, oe::modifiers mods) {
		if (!m_selected) return;
		char character = codepoint;

		if (m_filtering) {
			if (text_input_info.filter.find(character) == text_input_info.filter.npos)
				return;
		}

		stb_textedit_key(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state), character);
		resetTimer();

		if (text_input_info.callback_changed) text_input_info.callback_changed(text_input_info.text);
	}

	void TextInput::cursor(oe::mouse_buttons button, oe::actions action, const glm::vec2& cursor_window) {
		if (button == oe::mouse_buttons::button_left && action == oe::actions::press) {
			if (cursor_window.x >= render_position.x &&
				cursor_window.x < render_position.x + size.x &&
				cursor_window.y >= render_position.y &&
				cursor_window.y < render_position.y + size.y
			/*if (*/ )
			{
				m_selected = true;
				resetTimer();
			}
			else
			{
				m_selected = false;
			}
		}
	}

	void TextInput::key(oe::keys key, oe::actions action, oe::modifiers mods) {
		if (!m_selected) return;

		if (action != oe::actions::release) {
			if (key == oe::keys::key_backspace)
				stb_textedit_key(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state), STB_TEXTEDIT_K_BACKSPACE);
			else if (key == oe::keys::key_delete)
				stb_textedit_key(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state), STB_TEXTEDIT_K_DELETE);
			else if (key == oe::keys::key_left && mods == oe::modifiers::none)
				stb_textedit_key(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state), STB_TEXTEDIT_K_LEFT);
			else if (key == oe::keys::key_left && mods == oe::modifiers::control)
				stb_textedit_key(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state), STB_TEXTEDIT_K_WORDLEFT);
			else if (key == oe::keys::key_right && mods == oe::modifiers::none)
				stb_textedit_key(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state), STB_TEXTEDIT_K_RIGHT);
			else if (key == oe::keys::key_right && mods == oe::modifiers::control)
				stb_textedit_key(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state), STB_TEXTEDIT_K_WORDRIGHT);
			else if (key == oe::keys::key_v && mods == oe::modifiers::control) { // ctrl + v
				std::string cb = text_input_info.window_handle->getClipboard();
				stb_textedit_paste(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state), cb.c_str(), cb.size());
			}
			else if (key == oe::keys::key_c && mods == oe::modifiers::control) { // ctrl + c
				std::string copied = text_input_info.text.substr(reinterpret_cast<STB_TexteditState*>(m_state)->select_start, reinterpret_cast<STB_TexteditState*>(m_state)->select_end - reinterpret_cast<STB_TexteditState*>(m_state)->select_start);
				spdlog::info(copied);
				text_input_info.window_handle->setClipboard(copied);
				stb_textedit_cut(&text_input_info.text, reinterpret_cast<STB_TexteditState*>(m_state));
			}
			else if (key == oe::keys::key_enter) {
				if (text_input_info.callback_newline) text_input_info.callback_newline(text_input_info.text);
			}

			resetTimer();
			if (text_input_info.callback_changed) text_input_info.callback_changed(text_input_info.text);
		}
	}

}