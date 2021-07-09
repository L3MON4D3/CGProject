#include "LaserAction.hpp"
#include "Globals.hpp"

const float ray_speed = 300;

unsigned int LaserAction::shader_program;
int LaserAction::pvm_mat_loc;
int LaserAction::color_loc;
std::shared_ptr<geometry> LaserAction::ray;

LaserAction::LaserAction(float from, float until, glm::mat4 model_transform, glm::vec4 color) :
	ObjectAction{from, until},
	model_transform{model_transform},
	active_transform{glm::identity<glm::mat4>()},
	color{color} { }

void LaserAction::activate(float t, glm::mat4 obj_transform) {
	active_transform = obj_transform;
	actual_start = t;
}

void LaserAction::render(float t, glm::mat4 vp_mat, glm::mat4) {
	// translate to "blaster", then move towards z (shooting direction),
	// then apply craft-transforms from the moment of shooting.
	glm::mat4 proj_view_trans = vp_mat
		* active_transform
		* glm::translate((t - actual_start) * ray_speed * glm::vec3(0,0,1))
		* model_transform
		* ray->transform;

	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj_view_trans));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(proj_view_trans));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUseProgram(shader_program);
	glUniform4fv(color_loc, 1, &color.x);

	glBindVertexArray(ray->vao);
	glDrawElements(GL_TRIANGLES, ray->vertex_count, GL_UNSIGNED_INT, (void*) 0);
}	
