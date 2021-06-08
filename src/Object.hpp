#include "mesh.hpp"
#include "util.hpp"

#include <glm/gtx/transform.hpp>
#include <tinysplinecxx.h>
#include <functional>

const glm::vec3 model_forw = glm::vec3(1,0,0);

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
				glm::vec3 forw = util::std2glm(curves[1].eval(t).result());
				return
					// rotate model_forw onto forw.
					glm::rotate(glm::dot(model_forw, forw), glm::cross(model_forw, forw)) *
					// translate to position.
					glm::translate(util::std2glm(curves[0].eval(t).result()));
			}
	);
	
	glm::mat4 get_model_mat(float t);
};
