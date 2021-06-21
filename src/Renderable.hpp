#pragma once

class Renderable {
public:
	virtual ~Renderable() {}
	virtual void render(float, glm::mat4) = 0;
};
