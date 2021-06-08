#include "util.hpp"

namespace util {
	glm::vec3 std2glm(std::vector<double> vec) {
		return glm::vec3(vec[0], vec[1], vec[2]);
	}
}
