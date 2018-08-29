#include "fontLoader.h"


namespace graphics {
	
	FontLoader::FontLoader(std::string fontPath, Shader &shader) {
		init(fontPath);
		mShader = &shader;

		glGenVertexArrays(1, &mVAO);
		glGenBuffers(1, &mVBO);
		glBindVertexArray(mVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	bool FontLoader::init(std::string fontPath) {
		FT_Library ft;
		if (FT_Init_FreeType(&ft)) {
			std::cout << "ERROR Initalizing FreeType!" << std::endl; 
			return false;
		}

		FT_Face face;
		if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
			std::cout << "ERROR loading font!" << std::endl; 
			return false;
		}

		FT_Set_Pixel_Sizes(face, 0, 256);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
		for (GLubyte c = 0; c < 128; c++) {
			//Load glyph
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				std::cout << "ERROR loading glyph for " << c << std::endl;
				continue;
			}

			//Generate texture
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
			
			//Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
			//Now store character for later use
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			};
			mCharacters.insert(std::pair<GLchar, Character>(c, character));
		}

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		return true;
	}

	void FontLoader::renderText(std::string text, float x, float y, float scaleX, float scaleY, glm::vec3 color)
	{
		// Activate corresponding render state
		mShader->enable();
		mShader->setUniform3f("color", color);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(mVAO);

		// Iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = mCharacters[*c];
			GLfloat xpos = x + ch.bearing.x * scaleX / 2560.0f;
			GLfloat ypos = y - ch.bearing.y * scaleY / 2560.0f;
			GLfloat w = ch.size.x * scaleX / 2560.0f;
			GLfloat h = ch.size.y * scaleY / 2560.0f;

			// Update VBO for each character
			GLfloat vertices[6][4] = {
				{ xpos,     ypos + h, 0.0, 1.0 },
				{ xpos,     ypos,     0.0, 0.0 },
				{ xpos + w, ypos,     1.0, 0.0 },
				{ xpos,     ypos + h, 0.0, 1.0 },
				{ xpos + w, ypos,     1.0, 0.0 },
				{ xpos + w, ypos + h, 1.0, 1.0 }
			};

			// Render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.textureID);

			// Update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, mVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);

			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.advance >> 6) * scaleX / 2560.0f; // Bitshift by 6 to get value in pixels (2^6 = 64)
		}

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}


}