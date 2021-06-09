#include "mesh.hpp"
#include "util.hpp"
#include <cmath>

#include <glm/gtx/transform.hpp>
#include <tinysplinecxx.h>
#include <functional>

const glm::vec3 model_forw = glm::vec3(0,0,1);
const glm::vec3 model_up = glm::vec3(0,1,0);

class Object {
private:
	const geometry &model;
	const std::vector<tinyspline::BSpline> curves;
	std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> model_func;
	
public:
	Object(
		const geometry & model,
		const std::vector<tinyspline::BSpline> curves,
		std::function<glm::mat4(float, std::vector<tinyspline::BSpline>)> model_func =
			[](float t, std::vector<tinyspline::BSpline> curves) {
				// assume curves[1] is curves[0]'.
				glm::vec3 forw = glm::normalize(util::std2glm(curves[1].eval(t).result()));
				glm::mat4 forw_rotate = glm::rotate(
						std::acos(glm::dot(model_forw, forw)),
						glm::normalize(glm::cross(model_forw, forw)));
				glm::vec3 new_up = forw_rotate * glm::vec4(util::gs1(forw, model_up), 1);
				glm::mat4 up_rotate = glm::rotate(
						std::acos(glm::dot(new_up, model_up)),
						forw);
				return
					// translate to position.
					glm::translate(util::std2glm(curves[0].eval(t).result())) *
					// rotate model_forw onto forw.
					up_rotate * forw_rotate;
			}
	);
	
	glm::mat4 get_model_mat(float t);
};
