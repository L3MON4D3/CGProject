#pragma once

#include <glm/glm.hpp>

class Skybox {
private:
	unsigned int skyboxVAO;
    unsigned int cubemapTexture;

public:
	static unsigned int shader_program;
    Skybox();		
    void render(glm::mat4);
};
