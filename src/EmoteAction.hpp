#include "ObjectAction.hpp"
#include <tinysplinecxx.h>

class EmoteAction : public ObjectAction {
private:
	std::shared_ptr<tinyspline::BSpline> pos_curve;
	std::shared_ptr<tinyspline::BSpline> time_curve;
	unsigned int vao;

public:
	static unsigned int shader;
	static int pvm_loc;
	EmoteAction(float t, std::shared_ptr<tinyspline::BSpline> pos_curve, std::shared_ptr<tinyspline::BSpline> time_curve);
	void render(float t, glm::mat4 vp_mat);
	void activate(float, glm::mat4) {};
};
