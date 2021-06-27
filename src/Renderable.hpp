#pragma once
#include "glm/matrix.hpp"

class Renderable {
public:
	virtual ~Renderable() {}
	virtual void render(float, glm::mat4) = 0;
};
