#pragma once

//#include <glm/glm.hpp>
#include "Renderable.hpp"

class Skybox : public Renderable {
private:
	unsigned int vao;


public:
	static unsigned int shader_program;
	static int proj_view_loc;
    static int proj_view_loc;

//	Skybox()		
    void render(float, glm::mat4);
};
