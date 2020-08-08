#include "random.hpp"

#include <cstdlib>
#include <ctime>

namespace oe::utils {

	Random* Random::singleton = nullptr;

	void Random::seed(uint32_t value)
	{
		gen = std::mt19937(value);
	}

	glm::vec2 Random::randomVec2(float min, float max) {
		return glm::vec2(randomf(min, max), randomf(min, max));
	}

	glm::vec3 Random::randomVec3(float min, float max) {
		return glm::vec3(randomf(min, max), randomf(min, max), randomf(min, max));
	}

	glm::vec4 Random::randomVec4(float min, float max) {
		return glm::vec4(randomf(min, max), randomf(min, max), randomf(min, max), randomf(min, max));
	}

}