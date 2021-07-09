#include "ObjectAction.hpp"
#include <tinysplinecxx.h>

class EmoteAction : public ObjectAction {
private:
	std::shared_ptr<tinyspline::BSpline> pos_curve;
	unsigned int vao;
	unsigned int texture;

public:
	static unsigned int shader;
	static int pvm_loc;
	std::string image;
	EmoteAction(std::string image, float from, float until, std::shared_ptr<tinyspline::BSpline> pos_curve);
	void render(float t, glm::mat4 vp_mat, glm::mat4 model_mat);
	void activate(float, glm::mat4) {};
};
