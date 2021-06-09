#include "mesh.hpp"
#include "util.hpp"
#include <cmath>

#include <glm/gtx/transform.hpp>
#include <tinysplinecxx.h>
#include <functional>

const glm::vec3 model_forw = glm::vec3(0,0,1);
const glm::vec3 model_up = glm::vec3(0,1,0);
const glm::vec3 global_up = glm::vec3(0,1,0);

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
				glm::vec3 forw = glm::normalize(util::std2glm(curves[1].eval(t).result()));
				glm::vec3 up = util::gs1(forw, global_up);
				glm::vec3 x = glm::normalize(glm::cross(forw, up));
				// 'passive' rotation.
				glm::mat4 forw_rotate = {
					x.x, up.x, forw.x, 0,
					x.y, up.y, forw.y, 0,
					x.z, up.z, forw.z, 0,
					  0,    0,      0, 1,
				};

				return
					// translate to position.
					glm::translate(util::std2glm(curves[0].eval(t).result())) *
					// rotate model_forw onto forw.
					forw_rotate;
			}
	);
	
	glm::mat4 get_model_mat(float t);
};
