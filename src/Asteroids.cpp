#include "Asteroids.hpp"
#include "util.hpp"
#include "Globals.hpp"

const float RAND_MAX_F = float(RAND_MAX);

void Asteroids::init_asteroids() {
	size_t ast_count = models->size()*dups;
	asteroids = std::make_unique<asteroid[]>(ast_count);	
	positions = std::make_unique<float[]>(ast_count*3);	
	//rotations = std::make_unique<float[]>(ast_count);	

	for (size_t i = 0; i != ast_count; ++i) {
		// speed \in (-20,20)^3.
		asteroids[i].v = util::v3_rand() * 20.0f;
		//asteroids[i].rot_speed = std::rand()%10/100;

		float dist = radius_from + (std::rand()/RAND_MAX_F)*(radius_to - radius_from);
		asteroids[i].pos_start = glm::normalize(util::v3_rand()) * dist;

		asteroids[i].pos = &positions[3*i];
		asteroids[i].pos[0] = asteroids[i].pos_start[0];
		asteroids[i].pos[1] = asteroids[i].pos_start[1];
		asteroids[i].pos[2] = asteroids[i].pos_start[2];

		//asteroids[i].rot_amount = &rotations[i];
	}
}

// A bit spaghetti maybe.
Asteroids::Asteroids(std::shared_ptr<std::vector<geometry>> models, size_t model_count,	float radius_from, float radius_to, size_t dups) :
	models{models}, model_count{model_count}, radius_from{radius_from}, radius_to{radius_to}, dups{dups} {
	
	// inits positions also.
	init_asteroids();

	positions_vbo = std::make_unique<unsigned int[]>(model_count);
	//axis_vbo = std::make_unique<unsigned int[]>(model_count);
	//rot_vbo = std::make_unique<unsigned int[]>(model_count);
	vao = std::make_unique<unsigned int[]>(model_count);

	// init buffers.
	glGenVertexArrays(model_count, vao.get());
	glGenBuffers(model_count, positions_vbo.get());
	//glGenBuffers(model_count, axis_vbo.get());
	//glGenBuffers(model_count, rot_vbo.get());

	for (size_t i = 0; i != model_count; ++i) {
		glBindVertexArray(vao[i]);

		glBindBuffer(GL_ARRAY_BUFFER, positions_vbo[i]);
		glBufferData(GL_ARRAY_BUFFER, dups*3*sizeof(float), &positions[i*dups*3], GL_STATIC_DRAW);

		//auto data = std::make_unique<float[]>(dups*3);
		//for (size_t i = 0; i != dups; ++i) {
		//	glm::vec3 rot_ax = glm::normalize(util::v3_rand());
		//	data[i*3+0] = rot_ax[0];
		//	data[i*3+1] = rot_ax[1];
		//	data[i*3+2] = rot_ax[2];
		//}

		//glBindBuffer(GL_ARRAY_BUFFER, axis_vbo[i]);
		//glBufferData(GL_ARRAY_BUFFER, dups*3*sizeof(float), data.get(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, (*models)[i].vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3*sizeof(float)));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6*sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// pos
		glBindBuffer(GL_ARRAY_BUFFER, positions_vbo[i]);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0);
		glVertexAttribDivisor(3, 1);
		glEnableVertexAttribArray(3);

		//glBindBuffer(GL_ARRAY_BUFFER, axis_vbo[i]);
		//// axis
		//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0);
		//glVertexAttribDivisor(4, 1);
		//glEnableVertexAttribArray(4);

		//glBindBuffer(GL_ARRAY_BUFFER, rot_vbo[i]);
		//// amount
		//glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
		//glEnableVertexAttribArray(5);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*models)[i].ibo);
	}
}

void Asteroids::render(float t, glm::mat4 pv) {
	size_t ast_indx = 0;
	for (size_t i = 0; i != model_count; ++i) {
		for (size_t j = 0; j != dups; ++j, ++ast_indx)
			asteroids[ast_indx].update_pos(t);
		glBindBuffer(GL_ARRAY_BUFFER, positions_vbo[i]);
		glBufferData(GL_ARRAY_BUFFER, dups*3*sizeof(float),  &positions[i*3*dups], GL_STATIC_DRAW);

		//glBindBuffer(GL_ARRAY_BUFFER, rot_vbo[i]);
		//glBufferData(GL_ARRAY_BUFFER, dups*sizeof(float),  &positions[i*dups], GL_STATIC_DRAW);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pv));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUseProgram(Globals::shaders[Globals::shader_Asteroids]);
	for (size_t i = 0; i != model_count; ++i) {
		// model matrix for scaling.
		glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr((*models)[i].transform));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindVertexArray(vao[i]);
		glDrawElementsInstanced(GL_TRIANGLES, (*models)[i].vertex_count, GL_UNSIGNED_INT, 0, dups);
	}
}
