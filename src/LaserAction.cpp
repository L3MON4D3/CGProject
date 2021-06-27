#include "LaserAction.hpp"

const float ray_speed = 100;
const glm::vec4 color = glm::vec4(1,0,0,1);

unsigned int LaserAction::shader_program;
int LaserAction::pvm_mat_loc;
int LaserAction::color_loc;
std::shared_ptr<geometry> LaserAction::ray;

LaserAction::LaserAction(float start, glm::mat4 model_transform) :
	ObjectAction{start},
	model_transform{model_transform},
	active_transform{glm::identity<glm::mat4>()} { }

void LaserAction::activate(float t, Object &o) {
	active_transform = o.model_func(t, o);
	start_time = t;
}

void LaserAction::render(float t, glm::mat4 vp_mat) {
	glUseProgram(shader_program);
	glBindVertexArray(ray->vao);

	// translate to "blaster", then move towards z (shooting direction),
	// then apply craft-transforms from the moment of shooting.
	glm::mat4 proj_view_trans = vp_mat * model_transform
		* glm::translate((t - start_time) * ray_speed * glm::vec3(0,0,1))
		* active_transform;

	glUniformMatrix4fv(pvm_mat_loc, 1, GL_FALSE, &proj_view_trans[0][0]);
	glUniform4fv(color_loc, 1, &color.x);

	glDrawElements(GL_TRIANGLES, ray->vertex_count, GL_UNSIGNED_INT, (void*) 0);
}	
