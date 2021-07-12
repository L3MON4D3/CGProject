#include "util.hpp"
#include "Scene.hpp"
#include "LaserAction.hpp"
#include "EmoteAction.hpp"
#include "Globals.hpp"
#include "ExplodeAction.hpp"

#include "shader.hpp"

#include <memory>
#include <fstream>

const std::string exclamation = "data/PNG/Vector/Style 8/emote_exclamation.png";
const std::string exclamations = "data/PNG/Vector/Style 8/emote_exclamations.png";
const std::string question = "data/PNG/Vector/Style 8/emote_question.png";

const std::string particle = "data/particle.png";

namespace Loader {

void load_shader() {
	
	unsigned int vertexSkyboxShader = compileShader("skybox.vert", GL_VERTEX_SHADER);
    unsigned int fragmentSkyboxShader = compileShader("skybox.frag", GL_FRAGMENT_SHADER);
	Globals::shaderProgramObj = linkProgram(vertexSkyboxShader, fragmentSkyboxShader);
    glDeleteShader(vertexSkyboxShader);
    glDeleteShader(fragmentSkyboxShader);

	std::vector<std::string> files_base {"mesh_render", "curve_render", "quad_front", "mesh_render", "exhaust", "light", "particle"};

	for (unsigned int i = 0; i != Globals::shaders_sz; ++i) {
		unsigned int v = compileShader((files_base[i]+".vert").c_str(), GL_VERTEX_SHADER);
		unsigned int f = compileShader((files_base[i]+".frag").c_str(), GL_FRAGMENT_SHADER);
		Globals::shaders[i] = linkProgram(v, f);
		glDeleteShader(v);
		glDeleteShader(f);
		unsigned int indx = glGetUniformBlockIndex(Globals::shaders[i], "material");
		if (indx != GL_INVALID_INDEX)
			glUniformBlockBinding(Globals::shaders[i], indx, Globals::material_binding);

		indx = glGetUniformBlockIndex(Globals::shaders[i], "transforms");
		if (indx != GL_INVALID_INDEX)
			glUniformBlockBinding(Globals::shaders[i], indx, Globals::transforms_binding);

		indx = glGetUniformBlockIndex(Globals::shaders[i], "lighting");
		if (indx != GL_INVALID_INDEX)
			glUniformBlockBinding(Globals::shaders[i], indx, Globals::lighting_binding);
	}
}

void load_models() {
	Globals::cargo_A = std::make_shared<std::vector<geometry>>(loadScene("craft_cargoA.obj", false));
	(*Globals::cargo_A)[0].transform = glm::translate(glm::vec3(0, -.4, 0));

	Globals::laser_missile = std::make_shared<geometry>(loadScene("sphere.obj", false)[0]);
	Globals::laser_missile->transform = glm::scale(glm::vec3(.03, .03, .7));

	Globals::sphere = std::make_shared<geometry>(loadScene("sphere_fine.obj", false)[0]);
}

void load_ubos() {
	glGenBuffers(Globals::mat_sz, Globals::mat2ubo);
	for (unsigned int i = 0; i != Globals::mat_sz; ++i) {
		glBindBuffer(GL_UNIFORM_BUFFER, Globals::mat2ubo[i]);
		// space for 2 floats, 4-aligned, init with values stored in Globals.
		glBufferData(GL_UNIFORM_BUFFER, 8, Globals::material_values[i], GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	glGenBuffers(1, &Globals::transform_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, Globals::transform_ubo);
	// space for 2 4x4 matrices(float).
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), 0, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, Globals::transforms_binding, Globals::transform_ubo);

	glGenBuffers(1, &Globals::lighting_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, Globals::lighting_ubo);
	// space for 2 vec3, each 4-word-aligned.
	glBufferData(GL_UNIFORM_BUFFER, 2 * 4*4, 0, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, Globals::lighting_binding, Globals::lighting_ubo);

	glGenTextures(1, &Globals::particle_texture);
	glBindTexture(GL_TEXTURE_2D, Globals::particle_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(particle.c_str(), &width, &height, &channels, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
}

std::unique_ptr<Scene> load_scene1(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + ".curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines = util::read_splines(file, '#');
	file.close();

	file.open(filename + ".actions");
	std::vector<float> action_times = util::read_floats(file, '#');
	file.close();

	file.open(filename + ".light");
	std::vector<float> light = util::read_floats(file, '#');
	glm::vec3 light_glm {light[0], light[1], light[2]};
	file.close();

	file.open(filename + ".length");
	std::vector<float> length = util::read_floats(file, '#');
	file.close();

	glm::vec4 c1 = glm::vec4(1,0,0,1);
	glm::vec4 c2 = glm::vec4(0,1,0,1);

	auto spline_indx = splines.begin();
	auto time_indx = action_times.begin();

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		Globals::cargo_A,
		*spline_indx++, *spline_indx++, std::vector{
			*spline_indx++,
			*spline_indx++
		},
		std::vector<std::shared_ptr<ObjectAction>>{std::make_shared<LaserAction>(*time_indx++, *time_indx++, glm::identity<glm::mat4>(), c1)},
		Globals::cargo_A_shaders, Globals::cargo_A_ubos
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::cargo_A,
		*spline_indx++, *spline_indx++, std::vector{
			*spline_indx++,
			*spline_indx++
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(*time_indx++, *time_indx++, Globals::cargo_A_laser_origin_left, c2),
			std::make_shared<LaserAction>(*time_indx++, *time_indx++, Globals::cargo_A_laser_origin_right, c2),
			std::make_shared<LaserAction>(*time_indx++, *time_indx++, Globals::cargo_A_laser_origin_left, c2),
			std::make_shared<LaserAction>(*time_indx++, *time_indx++, Globals::cargo_A_laser_origin_right, c2),
			std::make_shared<EmoteAction>(question, *time_indx++, *time_indx++),
			std::make_shared<EmoteAction>(exclamation, *time_indx++, *time_indx++),
			std::make_shared<EmoteAction>(exclamation, *time_indx++, *time_indx++),
			std::make_shared<ExplodeAction>(.1, 1),
		},
		Globals::cargo_A_shaders, Globals::cargo_A_ubos
	));

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r, std::vector<char> render_curves) : ImGuiState{render_curves}, indx_rot{i_r}, range_rot{r_r} { }
	};

	return std::make_unique<Scene>(filename, std::move(objs), Camera{*spline_indx++, *spline_indx++, *spline_indx++, *spline_indx++, *spline_indx++, std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &scene) {
		auto state_cast = dynamic_cast<state1 *>(scene.state.get());
		Object &o = *scene.objects[state_cast->current_indx];
		tinyspline::BSpline &time_curve = *o.time_curve;
		ImGui::Begin("Obj_Rotation");
		util::plot_spline(*o.curves[1], "rot", [time_curve](const tinyspline::BSpline &spline, float t) {
			return spline.bisect(util::eval_timespline(time_curve, t)).result()[1];
		});
		util::control_point_edit(&o.curves[1],
			&state_cast->indx_rot, &state_cast->range_rot, state_cast->rot_offset);
		ImGui::End();

		o.curves[0] = std::make_shared<tinyspline::BSpline>(o.pos_curve->derive());
	}, std::make_unique<state1>(0,3, std::vector<char>{1, 1, 1, 1}), length[0], cam, light_glm);
}

std::unique_ptr<Scene> load_scene2(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + ".curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines = util::read_splines(file, '#');
	file.close();

	file.open(filename + ".length");
	std::vector<float> length = util::read_floats(file, '#');
	file.close();

	glm::vec3 zone {30,30,30};
	auto objs = std::vector<std::unique_ptr<Object>>();
	for (int i = 0; i != 20; ++i) {
		std::shared_ptr<std::vector<geometry>> asteroid = std::make_shared<std::vector<geometry>>();
		asteroid->emplace_back(*util::create_asteroid());
		float f = std::rand()/float(RAND_MAX)*2.5+0.8;
		(*asteroid)[0].transform = glm::scale(glm::vec3{f,f,f});
		auto pos_spline = std::make_shared<tinyspline::BSpline>(2, 3, 1);
		glm::vec3 p_1 = util::v3_rand()*zone;
		glm::vec3 p_2 = p_1+util::v3_rand()*glm::vec3(20,20,20);
		pos_spline->setControlPoints({
			p_1.x,
			p_1.y,
			p_1.z,
			p_2.x,
			p_2.y,
			p_2.z
		});

		auto rot_vec = std::make_shared<tinyspline::BSpline>(1, 3, 0);
		rot_vec->setControlPoints(util::glm2std(glm::normalize(util::v3_rand())));

		auto rot_speed = std::make_shared<tinyspline::BSpline>(1, 1, 0);
		rot_speed->setControlPoints(std::vector<double>{double(std::rand()%100)/10});

		objs.emplace_back(std::make_unique<Object>(
			asteroid,
			pos_spline, splines[1], std::vector<std::shared_ptr<tinyspline::BSpline>>{rot_vec, rot_speed},
			std::vector<std::shared_ptr<ObjectAction>>{},
			Globals::asteroid_shaders, Globals::asteroid_ubos, [](float t, Object &o) {
				return glm::translate(util::std2glm(o.pos_curve->eval(t).result()))
					*glm::rotate(float(t*o.curves[1]->eval(0).result()[0]), util::std2glm(o.curves[0]->eval(0).result()));
			}
		));
	}

	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines[4], splines[5], splines[6], splines[7], splines[8], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &) { }, std::make_unique<ImGuiState>(std::vector<char>(objs.size()+2)), length[0], cam);
}

void store_scene1(Scene &scene, std::string filename) {
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines;
	std::vector<float> actions;
	for (auto &obj : scene.objects) {
		splines.push_back(obj->pos_curve);
		splines.push_back(obj->time_curve);
		splines.insert(splines.end(), obj->curves.begin(), obj->curves.end());

		auto action_to_timepoint = [&actions](std::shared_ptr<ObjectAction> &act) {
			actions.push_back(act->from);
			actions.push_back(act->until);
		};
		std::for_each(obj->inactive.begin(), obj->inactive.end(), action_to_timepoint);
		std::for_each(obj->active.begin(), obj->active.end(), action_to_timepoint);
	}
	splines.push_back(scene.cam.pos_curve);
	splines.push_back(scene.cam.time_pos_curve);
	splines.push_back(scene.cam.look_curve);
	splines.push_back(scene.cam.time_look_curve);
	splines.push_back(scene.cam.zoom_curve);
	splines.insert(splines.end(), scene.cam.curves.begin(), scene.cam.curves.end());

	std::ofstream file;
	file.open(filename + ".curves");
	util::write_splines(splines, file, '#');
	file.close();

	file.open(filename + ".actions");
	util::write_floats(actions, file, '#');
	file.close();

	file.open(filename + ".light");
	std::vector<float> light_std{scene.light_dir.x,scene.light_dir.y,scene.light_dir.z};
	util::write_floats(light_std, file, '#');
	file.close();

	file.open(filename + ".length");
	std::vector<float> len_vec{float(scene.length)};
	util::write_floats(len_vec, file, '#');
	file.close();
}

}
