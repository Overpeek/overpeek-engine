#include "textLabel.hpp"

#include <sstream>
#include <iostream>
#include <ostream>

#include "font.hpp"
#include "engine/engine.hpp"
#include "engine/graphics/interface/window.hpp"
#include "engine/graphics/interface/framebuffer.hpp"
#include "engine/graphics/renderer.hpp"
#include "engine/asset/font_shader/font_shader.hpp"



namespace oe::graphics {
	
	class TextLabelRenderer
	{
	private:
		static TextLabelRenderer* singleton;
		TextLabelRenderer(const TextLabelRenderer& copy) = delete;
		TextLabelRenderer()
			: fb_shader()
			, fb_renderer(2048)
		{}

	public:
		static TextLabelRenderer& getSingleton() { if(!singleton) singleton = new TextLabelRenderer(); return *singleton; }

		oe::asset::FontShader fb_shader;
		oe::graphics::Renderer fb_renderer;
	};
	TextLabelRenderer* TextLabelRenderer::singleton = nullptr;



	template<typename char_type>
	void BasicTextLabel<char_type>::generate(const string_t& text, const Window& window, const oe::color& bg_color, float width, float outline_width, const oe::color& outline_c)
	{
		if (m_text == text && m_framebuffer) return; // why render the same text again?
		regenerate(text, window, bg_color, width, outline_width, outline_c);
	}
	
	template<typename char_type>
	void BasicTextLabel<char_type>::regenerate(const string_t& text, const Window& window, const oe::color& bg_color, float width, float outline_width, const oe::color& outline_c)
	{
		m_text = text;
		text_render_cache cache;
		text_t::create_text_render_cache(cache, m_text, m_font, { 0.0f, 0.0f }, glm::vec2(static_cast<float>(m_resolution)), oe::alignments::top_left);

		// size of the framebuffer
		m_size = cache.size;
		m_size.x = std::max(m_size.x, 1.0f);
		m_size.y = std::max(m_size.y, 1.0f);

		if (!m_framebuffer || m_fb_size.x < m_size.x || m_fb_size.y < m_size.y)
		{
			// create new framebuffer
			oe::FrameBufferInfo fb_info = {};
			fb_info.size = static_cast<glm::ivec2>(m_size) * 2; // double the required size, to make room for future reuse
			m_framebuffer = FrameBuffer(fb_info);
			
			m_fb_size = fb_info.size;
		}

		// bind fb
		m_framebuffer->bind();
		m_framebuffer->clear(bg_color);
		const glm::mat4 pr_matrix = glm::ortho(0.0f, m_fb_size.x, m_fb_size.y, 0.0f);

		// submit to the renderer render
		auto& tbr = TextLabelRenderer::getSingleton();
		text_t::submit(cache, tbr.fb_renderer);

		// render to the framebuffer
		tbr.fb_shader.setProjectionMatrix(pr_matrix);
		tbr.fb_shader.setSDF(m_font.isSDF());
		tbr.fb_shader.setWidth(width);
		tbr.fb_shader.setOutlineWidth(outline_width);
		tbr.fb_shader.setOutlineColor(outline_c);
		tbr.fb_renderer.render();
		tbr.fb_renderer.clear();

		// bind window framebuffer
		window->bind();

		// generate the sprite for user
		const glm::vec2 size_ratio = m_size / m_fb_size;

		m_sprite.m_owner = m_framebuffer->getTexture();
		m_sprite.position = { 0.0f, 1.0f };
		m_sprite.size = { size_ratio.x, -size_ratio.y };
		m_sprite.opacity = true;
	}
	


	template<typename char_type>
	void BasicText<char_type>::create_text_render_cache(text_render_cache& cache, const string_t& text, Font& font, const glm::vec2& origin_pos, const glm::vec2& size, const glm::vec2& align_to_origin)
	{
		// at the very least the size of the text
		// \0, \n, etc... will be removed
		size_t cache_size = std::accumulate(text.vec.cbegin(), text.vec.cend(), static_cast<size_t>(0), [](size_t since, const oe::utils::color_string_part<char_type>& part){ return since + std::get<0>(part).size(); });
		cache.datapoints = {};
		cache.datapoints.reserve(cache_size);

		glm::vec2 advance = origin_pos;
		char32_t codepoint = U'\0';
		char32_t codepoint_next = U'\0';

		for (size_t j = 0; j < text.vec.size(); j++)
		{
			const auto& string_color_part = text.vec.at(j);
			const auto& string_part = std::get<0>(string_color_part);
			const auto& color_part = std::get<1>(string_color_part);

			for (size_t i = 0; i < string_part.size() + 1; i++)
			{

				// get the codepoint from the input text at i
				codepoint = codepoint_next;
				// and the next codepoint or \0
				codepoint_next = U'\0';
				if(i != string_part.size())
					codepoint_next = static_cast<char32_t>(string_part.at(i));

				// skip the first
				if(i == 0)
					continue;
				
				auto& datapoint = cache.datapoints.emplace_back();

				// test if the codepoint will be rendered
				// and advancing the positioning of the following characters
				datapoint.rendered = false;
				if(codepoint == U'\n') // new line
				{
					advance.x = origin_pos.x;
					advance.y += size.y;
					continue;
				}
				else if(codepoint == U'\t') // tab
				{
					advance.x = std::ceil(advance.x / size.x) * size.x;
					continue;
				}
				else if(codepoint == U'\0') // null terminator
					continue;

				// get glyph, if possible
				const oe::graphics::Font::Glyph* glyph = font.getGlyph(codepoint);
				// get 0 glyph as the codepoint did not contain any glyphs
				if (!glyph) glyph = font.getGlyph(0);
				// unexpectedly no 0 glyph
				if (!glyph) throw std::runtime_error("Unexpected NULL glyph at slot 0");

				// insert new datapoint
				datapoint.rendered = true;
				datapoint.codepoint = codepoint;
				datapoint.color = color_part;
				datapoint.position = (advance) + (glyph->top_left * size) + glm::vec2{ 0.0f, -font.m_topmost * size.y };
				datapoint.size = glyph->size * size;
				datapoint.sprite = glyph->sprite;

				// advancing
				advance += glm::vec2(glyph->advance.x, 0.0f) * size;
				// kerning
				advance.x += font.getKerning(codepoint, codepoint_next) * size.x;
			}
		}

		// alignment
		// find topleft and bottomright
		glm::vec2 min = origin_pos, max = origin_pos;
		for (auto& datapoint : cache.datapoints)
		{
			min = glm::min(min, datapoint.position);
			max = glm::max(max, datapoint.position + datapoint.size);
		}
		// move all datapoints
		const glm::vec2 total_size = glm::abs(max - min);
		const glm::vec2 align_move = oe::alignmentOffset(total_size, align_to_origin);
		if(align_to_origin != oe::alignments::top_left || min != origin_pos)
			for (auto& datapoint : cache.datapoints)
				datapoint.position -= align_move;
		
		// extra cache data
		cache.top_left = min;
		cache.size = total_size;
	}

	template<typename char_type>
	glm::vec2 BasicText<char_type>::offset_to_char(const text_render_cache& cache, size_t index)
	{
		if(index >= cache.datapoints.size())
			return { 0.0f, 0.0f };

		return cache.datapoints.back().position - cache.datapoints.front().position;
	}

	template<typename char_type>
	void BasicText<char_type>::submit(const text_render_cache& cache, Renderer& renderer)
	{
		for (const auto& datapoint : cache.datapoints)
		{
			if(!datapoint.rendered)
				continue;

			auto quad = renderer.create();
			quad->setPosition(datapoint.position);
			quad->setSize(datapoint.size);
			quad->setSprite(datapoint.sprite);
			quad->setColor(datapoint.color);
			
			renderer.forget(std::move(quad));
		}
	}
	


	template class BasicTextLabel<char>;
	template class BasicTextLabel<wchar_t>;
	template class BasicTextLabel<char16_t>;
	template class BasicTextLabel<char32_t>;
	
	template class BasicText<char>;
	template class BasicText<wchar_t>;
	template class BasicText<char16_t>;
	template class BasicText<char32_t>;
}