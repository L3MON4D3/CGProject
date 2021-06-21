#include <tinysplinecxx.h>
#include <glm/glm.hpp>
#include "Renderable.hpp"

class Curve : public Renderable {
private:
	unsigned int vao;

public:
	static unsigned int shader_program;
	static unsigned int proj_view_loc;
	static int color_loc;
	Curve(
		tinyspline::BSpline,
		glm::vec4 color = glm::vec4(1.0,0.0,0.0,1.0));
	void set_color(glm::vec4);
	void render(float, glm::mat4);
};
