#include <tinysplinecxx.h>
#include <glm/glm.hpp>
#include "Renderable.hpp"

class Curve : public Renderable {
private:
	unsigned int vao;
	unsigned int shader_program;
	int color_loc;

public:
	glm::vec4 color;
	Curve(
		tinyspline::BSpline,
		unsigned int shader_program,
		glm::vec4 color = glm::vec4(1.0,0.0,0.0,1.0));
	void render(float);
};
