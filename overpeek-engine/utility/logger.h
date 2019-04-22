#pragma once

#include <string>
#include <glm/glm.hpp>

namespace oe {
	//not in logger
	static enum outType {
		info, warning, critical, error
	};

	class Logger {
	private:
		static void *m_console;
		static bool m_initalized;

	public:

		static void setup();

		static void out(int type, const char *output);
		static void out(int type, std::string output);
		static void out(int type, double output);
		static void out(int type, const char *output, const char *output2);
		static void out(int type, const char *output, double output2);
		static void out(int type, const char *output, double output2, double output3);
		static void out(int type, const char *output, const char *output2, const char *output3);
		static void out(int type, const char *output, double output2, const char *output3, double output4);
		static void out(int type, const char *output, double output2, const char *output3, double output4, const char *output5, double output6);

		static void out(int type, glm::vec2 output);
		static void out(int type, const char* output, glm::vec2 output2);

		static void out(int type, glm::vec3 output);
		static void out(int type, const char* output, glm::vec3 output2);

		static void out(int type, glm::vec4 output);
		static void out(int type, const char* output, glm::vec4 output2);

	};
}