#pragma once

#include "engine/internal_libs.hpp"
#include "engine/enum.hpp"
#include "engine/utility/fileio.hpp"

#include <string>
#include <unordered_map>



namespace oe::graphics { struct Sprite; class SpritePack; }

namespace oe::graphics
{
	struct FontData;

	class Font
	{
	public:
		struct Glyph {
			char32_t codepoint;
			glm::vec2 size;
			glm::vec2 top_left;
			glm::vec2 advance;

			const Sprite* sprite;
		};
		float m_topmost;

	private:
		SpritePack* m_sprite_pack = nullptr;
		std::unordered_map<char32_t, Glyph> m_glyphs{};
		FontData* m_data = nullptr;

		uint16_t m_resolution{};
		bool m_sdf = false;
		oe::utils::byte_string m_font_file{};
		
		glm::vec2 m_avg_size{ 0.0f, 0.0f };
		float m_avg_size_c = 0.0f;

		bool gen_codepoint_glyph(char32_t codepoint);
	
	public:
		/*
		font_path = "default" loads (Arial.ttf/LiberationSans-Regular.ttf) from
		linux: /usr/share/fonts/truetype/liberation/
		windows: C:/Windows/Fonts/
		macos: *workdir*
		*/
		Font(uint16_t resolution = 64u, bool sdf = false, const oe::utils::FontFile& font_file = {});
		~Font();

		const Glyph* getGlyph(char32_t c);
		float getKerning(char32_t c, char32_t c_next) const;
		[[nodiscard]] constexpr inline uint16_t getResolution() const noexcept { return m_resolution; }
		[[nodiscard]] constexpr inline SpritePack* getSpritePack() noexcept { return m_sprite_pack; }
		[[nodiscard]] constexpr inline bool isSDF() const noexcept { return m_sdf; }
		[[nodiscard]] constexpr inline const glm::vec2& avgSize() const noexcept { return m_avg_size; }
	};

}

