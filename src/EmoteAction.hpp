#include "ObjectAction.hpp"
#include <tinysplinecxx.h>

class EmoteAction : public ObjectAction {
private:
	unsigned int vao;
	unsigned int texture;
	std::string image;
	float scale, height;

public:
	static unsigned int shader;
	static int pvm_loc;
	EmoteAction(std::string image, float from, float until, float scale=2, float height=1.6);
	void render(float t, glm::mat4 vp_mat, glm::mat4 model_mat);
	void activate(float, glm::mat4) {};
};
