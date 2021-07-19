#include "util.hpp"
#include "Scene.hpp"
#include "LaserAction.hpp"
#include "EmoteAction.hpp"
#include "Globals.hpp"
#include "ExplodeAction.hpp"
#include "TurbineAction.hpp"

#include "shader.hpp"

#include <memory>
#include <fstream>

const float RAND_MAX_F = float(RAND_MAX);

const std::string exclamation = "data/PNG/Vector/Style 8/emote_exclamation.png";
const std::string exclamations = "data/PNG/Vector/Style 8/emote_exclamations.png";
const std::string question = "data/PNG/Vector/Style 8/emote_question.png";
const std::string radio = "data/radio.png";
const std::string treasure = "data/treasure.png";
const std::string treasure2 = "data/treasure2.png";

const std::string particle = "data/particle.png";

namespace Loader {

void load_shader() {
	std::vector<std::string> files_base {"mesh_render", "curve_render", "quad_front", "glass", "exhaust", "light", "particle", "skybox", "asteroids", "turbine"};

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

	Globals::pirate = std::make_shared<std::vector<geometry>>(loadScene("pirate.obj", false));
	(*Globals::pirate)[0].transform = glm::translate(glm::vec3(0, -.25, 0));

	Globals::support = std::make_shared<std::vector<geometry>>(loadScene("support.obj", false));
	(*Globals::pirate)[0].transform = glm::translate(glm::vec3(0, -.25, 0));

	Globals::station = std::make_shared<std::vector<geometry>>(loadScene("station2.obj", false));
	(*Globals::station)[0].transform = glm::scale(glm::vec3(20,20,20));

	Globals::laser_missile = std::make_shared<geometry>(loadScene("sphere.obj", false)[0]);
	Globals::laser_missile->transform = glm::scale(glm::vec3(.03, .03, .7));

	Globals::sphere = std::make_shared<geometry>(loadScene("sphere_fine.obj", false)[0]);

	const size_t asteroid_count = 200;
	Globals::asteroids = std::make_shared<std::vector<geometry>>();
	Globals::asteroids->reserve(asteroid_count);
	for (int i = 0; i != asteroid_count; ++i) {
		geometry aster = *util::create_asteroid();

		float f = std::rand()/float(RAND_MAX)*2.5+0.8;
		aster.transform = glm::scale(glm::vec3{f,f,f});

		(*Globals::asteroids).push_back(aster);
	}
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

    // load textures
    std::vector<std::string> faces{
        "data/right_1.png",
        "data/left_1.png",
        "data/top_1.png",
        "data/bottom_1.png",
        "data/back_1.png",
        "data/front_1.png"
    };
	Globals::skybox = util::loadCubemap(faces);  
}

std::unique_ptr<Scene> load_scene1(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + "-0.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_0 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-1.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_1 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-cam.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_cam = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-0.actions");
	std::vector<float> actions_0 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-1.actions");
	std::vector<float> actions_1 = util::read_floats(file, '#');
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

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		Globals::cargo_A,
		splines_0[0], splines_0[1], std::vector{
			splines_0[2],
			splines_0[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(actions_0[0], actions_0[1], glm::identity<glm::mat4>(), c1),

			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_right, 0, 1),
		},
		Globals::cargo_A_shaders, Globals::cargo_A_ubos
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::station,
		splines_1[0], splines_1[1], std::vector{
			splines_1[2],
			splines_1[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(actions_1[0], actions_1[1], Globals::cargo_A_laser_origin_left, c2),
			std::make_shared<LaserAction>(actions_1[2], actions_1[3], Globals::cargo_A_laser_origin_right, c2),
			std::make_shared<LaserAction>(actions_1[4], actions_1[5], Globals::cargo_A_laser_origin_left, c2),
			std::make_shared<LaserAction>(actions_1[6], actions_1[7], Globals::cargo_A_laser_origin_right, c2),
			std::make_shared<EmoteAction>(question, actions_1[8], actions_1[9]),
			std::make_shared<EmoteAction>(exclamation, actions_1[10], actions_1[11]),
			std::make_shared<EmoteAction>(exclamation, actions_1[12], actions_1[13]),
			std::make_shared<ExplodeAction>(.1, 1),
		},
		Globals::station_shaders, Globals::station_ubos
	));

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r, std::vector<char> render_curves) : ImGuiState{render_curves}, indx_rot{i_r}, range_rot{r_r} { }
	};

	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines_cam[0], splines_cam[1], splines_cam[2], splines_cam[3], splines_cam[4], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &scene) {
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

std::unique_ptr<Scene> load_station(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + "-0.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_0 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-1.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_1 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-cam.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_cam = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-0.actions");
	std::vector<float> actions_0 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-1.actions");
	std::vector<float> actions_1 = util::read_floats(file, '#');
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

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		Globals::cargo_A,
		splines_0[0], splines_0[1], std::vector{
			splines_0[2],
			splines_0[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_right, 0, 1),
		},
		Globals::cargo_A_shaders, Globals::cargo_A_ubos, [](float t, float t_lin, Object &o) {
				if (t_lin >= 0.97)
					return glm::zero<glm::mat4>();
				// Calculate correct forward from derived func.
				glm::vec3 forw = glm::normalize(util::std2glm(o.curves[0]->eval(t).result()));
				// get vector that points up and is orthogonal to forw.
				glm::vec3 up = util::gs1(forw, util::up);
				// Third vector for complete base.
				glm::vec3 x = glm::normalize(glm::cross(forw, up));

				glm::mat4 rot = {
					   x.x,    x.y,    x.z, 0,
					  up.x,   up.y,   up.z, 0,
					forw.x, forw.y, forw.z, 0,
					     0,      0,      0, 1
				};

				return
					// translate to position.
					glm::translate(util::std2glm(o.pos_curve->eval(t).result())) *
					// rotate model_forw onto forw
					rot * glm::rotate<float>(o.curves[1]->bisect(t_lin).result()[1], glm::vec3(0,0,1));
			}
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::station,
		splines_1[0], splines_1[1], std::vector{splines_1[2], splines_1[3]},
		std::vector<std::shared_ptr<ObjectAction>>{},
		Globals::station_shaders, Globals::station_ubos, [](float t, float _, Object &o) {
			return glm::translate(util::std2glm(o.pos_curve->eval(t).result())) *
				glm::rotate<float>(M_PI/2+t, glm::vec3(0,1,0));
		}
	));

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r, std::vector<char> render_curves) : ImGuiState{render_curves}, indx_rot{i_r}, range_rot{r_r} { }
	};

	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines_cam[0], splines_cam[1], splines_cam[2], splines_cam[3], splines_cam[4], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &scene) {
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

std::unique_ptr<Scene> load_travel(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + "-0.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_0 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-1.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_1 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-2.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_2 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-cam.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_cam = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-0.actions");
	std::vector<float> actions_0 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-1.actions");
	std::vector<float> actions_1 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-2.actions");
	std::vector<float> actions_2 = util::read_floats(file, '#');
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

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		Globals::support,
		splines_0[0], splines_0[1], std::vector{
			splines_0[2],
			splines_0[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<EmoteAction>(treasure, actions_0[0], actions_0[1], 10, 4),
			std::make_shared<EmoteAction>(treasure2, actions_0[2], actions_0[3], 10, 4),
			std::make_shared<EmoteAction>(treasure, actions_0[4], actions_0[5], 10, 4),
			std::make_shared<EmoteAction>(treasure2, actions_0[6], actions_0[7], 10, 4),
			std::make_shared<EmoteAction>(treasure, actions_0[8], actions_0[9], 10, 4),

			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_right, 0, 1),
		},
		Globals::support_shaders, Globals::support_ubos,
			[](float t, float t_lin, Object &o) {
				if (t_lin <= 0.05)
					return glm::zero<glm::mat4>();
				// Calculate correct forward from derived func.
				glm::vec3 forw = glm::normalize(util::std2glm(o.curves[0]->eval(t).result()));
				// get vector that points up and is orthogonal to forw.
				glm::vec3 up = util::gs1(forw, util::up);
				// Third vector for complete base.
				glm::vec3 x = glm::normalize(glm::cross(forw, up));

				glm::mat4 rot = {
					   x.x,    x.y,    x.z, 0,
					  up.x,   up.y,   up.z, 0,
					forw.x, forw.y, forw.z, 0,
					     0,      0,      0, 1
				};

				return
					// translate to position.
					glm::translate(util::std2glm(o.pos_curve->eval(t).result())) *
					// rotate model_forw onto forw
					rot * glm::rotate<float>(o.curves[1]->bisect(t_lin).result()[1], glm::vec3(0,0,1));
			}
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::pirate,
		splines_1[0], splines_1[1], std::vector{
			splines_1[2],
			splines_1[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<TurbineAction>(Globals::pirate_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::pirate_turbine_right, 0, 1),
		},
		Globals::pirate_shaders, Globals::pirate_ubos
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::pirate,
		splines_2[0], splines_2[1], std::vector{
			splines_2[2],
			splines_2[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<TurbineAction>(Globals::pirate_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::pirate_turbine_right, 0, 1),
		},
		Globals::pirate_shaders, Globals::pirate_ubos
	));

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r, std::vector<char> render_curves) : ImGuiState{render_curves}, indx_rot{i_r}, range_rot{r_r} { }
	};

	std::vector<char> render_curves(objs.size()+2);
	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines_cam[0], splines_cam[1], splines_cam[2], splines_cam[3], splines_cam[4], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &scene) {
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
	}, std::make_unique<state1>(0,3, render_curves), length[0], cam, light_glm);
}

std::unique_ptr<Scene> load_chase(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + "-0.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_0 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-1.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_1 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-2.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_2 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-cam.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_cam = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-0.actions");
	std::vector<float> actions_0 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-1.actions");
	std::vector<float> actions_1 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-2.actions");
	std::vector<float> actions_2 = util::read_floats(file, '#');
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

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		Globals::cargo_A,
		splines_0[0], splines_0[1], std::vector{
			splines_0[2],
			splines_0[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<EmoteAction>(exclamation, actions_0[0], actions_0[1], 3, 1.8),
			std::make_shared<EmoteAction>(exclamation, actions_0[2], actions_0[3], 3, 1.8),

			std::make_shared<EmoteAction>(radio, actions_0[4], actions_0[5], 3, 1.8),
			std::make_shared<EmoteAction>(radio, actions_0[6], actions_0[7], 3, 1.8),
			std::make_shared<EmoteAction>(radio, actions_0[8], actions_0[9], 3, 1.8),

			std::make_shared<EmoteAction>(radio, actions_0[10], actions_0[11], 3, 1.8),
			std::make_shared<EmoteAction>(radio, actions_0[12], actions_0[13], 3, 1.8),
			std::make_shared<EmoteAction>(radio, actions_0[14], actions_0[15], 3, 1.8),

			std::make_shared<EmoteAction>(radio, actions_0[16], actions_0[17], 3, 1.8),
			std::make_shared<EmoteAction>(radio, actions_0[18], actions_0[19], 3, 1.8),
			std::make_shared<EmoteAction>(radio, actions_0[20], actions_0[21], 3, 1.8),

			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_right, 0, 1),
		},
		Globals::cargo_A_shaders, Globals::cargo_A_ubos,
			[](float t, float t_lin, Object &o) {
				// Calculate correct forward from derived func.
				glm::vec3 forw = glm::normalize(util::std2glm(o.curves[0]->eval(t).result()));
				// get vector that points up and is orthogonal to forw.
				glm::vec3 up = util::gs1(forw, util::up);
				// Third vector for complete base.
				glm::vec3 x = glm::normalize(glm::cross(forw, up));

				glm::mat4 rot = {
					   x.x,    x.y,    x.z, 0,
					  up.x,   up.y,   up.z, 0,
					forw.x, forw.y, forw.z, 0,
					     0,      0,      0, 1
				};

				return
					// translate to position.
					glm::translate(util::std2glm(o.pos_curve->eval(t).result())) *
					// rotate model_forw onto forw
					rot * glm::rotate<float>(o.curves[1]->bisect(t_lin).result()[1], glm::vec3(0,0,1));
			}
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::pirate,
		splines_1[0], splines_1[1], std::vector{
			splines_1[2],
			splines_1[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(actions_1[0], actions_1[1], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[2], actions_1[3], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[4], actions_1[5], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[6], actions_1[7], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[8], actions_1[9], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[10], actions_1[11], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[12], actions_1[13], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[14], actions_1[15], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[16], actions_1[17], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[18], actions_1[19], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[20], actions_1[21], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[22], actions_1[23], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[24], actions_1[25], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[26], actions_1[27], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[28], actions_1[29], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[30], actions_1[31], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[32], actions_1[33], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[34], actions_1[35], Globals::pirate_laser_origin_right, c1),

			std::make_shared<TurbineAction>(Globals::pirate_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::pirate_turbine_right, 0, 1),
		},
		Globals::pirate_shaders, Globals::pirate_ubos
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::pirate,
		splines_2[0], splines_2[1], std::vector{
			splines_2[2],
			splines_2[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(actions_2[0], actions_2[1], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[2], actions_2[3], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[4], actions_2[5], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[6], actions_2[7], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[8], actions_2[9], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[10], actions_2[11], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[12], actions_2[13], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[14], actions_2[15], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[16], actions_2[17], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[18], actions_2[19], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[20], actions_2[21], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[22], actions_2[23], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[24], actions_2[25], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[26], actions_2[27], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[28], actions_2[29], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[30], actions_2[31], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[32], actions_2[33], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[34], actions_2[35], Globals::pirate_laser_origin_right, c1),

			std::make_shared<TurbineAction>(Globals::pirate_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::pirate_turbine_right, 0, 1),
		},
		Globals::pirate_shaders, Globals::pirate_ubos
	));

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r, std::vector<char> render_curves) : ImGuiState{render_curves}, indx_rot{i_r}, range_rot{r_r} { }
	};

	std::vector<char> render_curves(objs.size()+2);
	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines_cam[0], splines_cam[1], splines_cam[2], splines_cam[3], splines_cam[4], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &scene) {
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
	}, std::make_unique<state1>(0,3, render_curves), length[0], cam, light_glm);
}

std::unique_ptr<Scene> load_asteroids_1(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + "-0.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_0 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-1.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_1 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-2.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_2 = util::read_splines(file, '#');
	file.close();

	std::vector<std::shared_ptr<tinyspline::BSpline>> asteroid_pos = {};
	std::vector<std::shared_ptr<tinyspline::BSpline>> asteroid_axis = {};
	std::vector<std::shared_ptr<tinyspline::BSpline>> asteroid_speed= {};

	asteroid_pos.reserve(Globals::asteroids->size());
	asteroid_axis.reserve(Globals::asteroids->size());
	asteroid_speed.reserve(Globals::asteroids->size());

	for (size_t i = 0; i != Globals::asteroids->size(); ++i) {
		file.open(filename +"-"+ std::to_string(i+3) + ".curves");
		auto ast_splines = util::read_splines(file, '#');
		asteroid_pos.push_back(ast_splines[0]);
		asteroid_axis.push_back(ast_splines[2]);
		asteroid_speed.push_back(ast_splines[3]);

		file.close();
	}

	file.open(filename + "-cam.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_cam = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-0.actions");
	std::vector<float> actions_0 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-1.actions");
	std::vector<float> actions_1 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-2.actions");
	std::vector<float> actions_2 = util::read_floats(file, '#');
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

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		Globals::cargo_A,
		splines_0[0], splines_0[1], std::vector{
			splines_0[2],
			splines_0[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_right, 0, 1),
		},
		Globals::cargo_A_shaders, Globals::cargo_A_ubos,
			[](float t, float t_lin, Object &o) {
				// Calculate correct forward from derived func.
				glm::vec3 forw = glm::normalize(util::std2glm(o.curves[0]->eval(t).result()));
				// get vector that points up and is orthogonal to forw.
				glm::vec3 up = util::gs1(forw, util::up);
				// Third vector for complete base.
				glm::vec3 x = glm::normalize(glm::cross(forw, up));

				glm::mat4 rot = {
					   x.x,    x.y,    x.z, 0,
					  up.x,   up.y,   up.z, 0,
					forw.x, forw.y, forw.z, 0,
					     0,      0,      0, 1
				};

				return
					// translate to position.
					glm::translate(util::std2glm(o.pos_curve->eval(t).result())) *
					// rotate model_forw onto forw
					rot * glm::rotate<float>(o.curves[1]->bisect(t_lin).result()[1], glm::vec3(0,0,1));
			}
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::pirate,
		splines_1[0], splines_1[1], std::vector{
			splines_1[2],
			splines_1[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(actions_1[0], actions_1[1], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[2], actions_1[3], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[4], actions_1[5], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[6], actions_1[7], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[8], actions_1[9], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[10], actions_1[11], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[12], actions_1[13], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[14], actions_1[15], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[16], actions_1[17], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[18], actions_1[19], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[20], actions_1[21], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[22], actions_1[23], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[24], actions_1[25], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[26], actions_1[27], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[28], actions_1[29], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[30], actions_1[31], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[32], actions_1[33], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[34], actions_1[35], Globals::pirate_laser_origin_right, c1),

			std::make_shared<TurbineAction>(Globals::pirate_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::pirate_turbine_right, 0, 1),
		},
		Globals::pirate_shaders, Globals::pirate_ubos
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::pirate,
		splines_2[0], splines_2[1], std::vector{
			splines_2[2],
			splines_2[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(actions_2[0], actions_2[1], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[2], actions_2[3], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[4], actions_2[5], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[6], actions_2[7], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[8], actions_2[9], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[10], actions_2[11], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[12], actions_2[13], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[14], actions_2[15], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[16], actions_2[17], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[18], actions_2[19], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[20], actions_2[21], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[22], actions_2[23], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[24], actions_2[25], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[26], actions_2[27], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[28], actions_2[29], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[30], actions_2[31], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[32], actions_2[33], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[34], actions_2[35], Globals::pirate_laser_origin_right, c1),

			std::make_shared<TurbineAction>(Globals::pirate_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::pirate_turbine_right, 0, 1),
		},
		Globals::pirate_shaders, Globals::pirate_ubos
	));

	//glm::vec3 zone {500,100,500};
	//glm::vec3 center {400,-20,500};
	for (size_t i = 0; i != Globals::asteroids->size(); ++i) {
		//float len = std::rand()/float(RAND_MAX);

		//glm::vec3 p_1 = center + glm::normalize(util::v3_rand())*zone*len;
		//glm::vec3 p_2 = p_1+util::v3_rand()*glm::vec3(20,20,20);

		//auto pos_spline = std::make_shared<tinyspline::BSpline>(2, 3, 1);
		//pos_spline->setControlPoints({
		//	p_1.x,
		//	p_1.y,
		//	p_1.z,
		//	p_2.x,
		//	p_2.y,
		//	p_2.z
		//});

		//auto rot_vec = std::make_shared<tinyspline::BSpline>(1, 3, 0);
		//rot_vec->setControlPoints(util::glm2std(glm::normalize(util::v3_rand())));

		//auto rot_speed = std::make_shared<tinyspline::BSpline>(1, 1, 0);
		//rot_speed->setControlPoints(std::vector<double>{double(std::rand()%100)/10});

		objs.emplace_back(std::make_unique<Object>(
			std::make_shared<std::vector<geometry>>(std::vector{(*Globals::asteroids)[i]}),
			asteroid_pos[i], splines_1[1], std::vector<std::shared_ptr<tinyspline::BSpline>>{asteroid_axis[i], asteroid_speed[i]},
			std::vector<std::shared_ptr<ObjectAction>>{},
			Globals::asteroid_shaders, Globals::asteroid_ubos, [](float t, float, Object &o) {
				return glm::translate(util::std2glm(o.pos_curve->eval(t).result()))
					*glm::rotate(float(t*o.curves[1]->eval(0).result()[0]), util::std2glm(o.curves[0]->eval(0).result()));
			}
		));
	}

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r, std::vector<char> render_curves) : ImGuiState{render_curves}, indx_rot{i_r}, range_rot{r_r} { }
	};

	std::vector<char> render_curves(objs.size()+2);
	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines_cam[0], splines_cam[1], splines_cam[2], splines_cam[3], splines_cam[4], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &scene) {
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
	}, std::make_unique<state1>(0,3, render_curves), length[0], cam, light_glm);
}

std::unique_ptr<Scene> load_asteroids_2(std::string filename, std::shared_ptr<camera> cam) {
	std::ifstream file;
	file.open(filename + "-0.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_0 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-1.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_1 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-2.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_2 = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-3.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> ast_big_pos = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-4.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> ast_big_pos2 = util::read_splines(file, '#');
	file.close();

	std::vector<std::shared_ptr<tinyspline::BSpline>> asteroid_pos = {};
	std::vector<std::shared_ptr<tinyspline::BSpline>> asteroid_time = {};
	std::vector<std::shared_ptr<tinyspline::BSpline>> asteroid_axis = {};
	std::vector<std::shared_ptr<tinyspline::BSpline>> asteroid_speed= {};

	asteroid_pos.reserve(Globals::asteroids->size());
	asteroid_time.reserve(Globals::asteroids->size());
	asteroid_axis.reserve(Globals::asteroids->size());
	asteroid_speed.reserve(Globals::asteroids->size());

	int dup_count = 6;
	for (size_t i = 0; i != Globals::asteroids->size()*dup_count; ++i) {
		file.open(filename +"-"+ std::to_string(i+5) + ".curves");
		auto ast_splines = util::read_splines(file, '#');
		asteroid_pos.push_back(ast_splines[0]);
		asteroid_time.push_back(ast_splines[1]);
		asteroid_axis.push_back(ast_splines[2]);
		asteroid_speed.push_back(ast_splines[3]);

		file.close();
	}


	file.open(filename + "-cam.curves");
	std::vector<std::shared_ptr<tinyspline::BSpline>> splines_cam = util::read_splines(file, '#');
	file.close();

	file.open(filename + "-0.actions");
	std::vector<float> actions_0 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-1.actions");
	std::vector<float> actions_1 = util::read_floats(file, '#');
	file.close();

	file.open(filename + "-2.actions");
	std::vector<float> actions_2 = util::read_floats(file, '#');
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

	auto objs = std::vector<std::unique_ptr<Object>>();
	objs.emplace_back(std::make_unique<Object>(
		Globals::cargo_A,
		splines_0[0], splines_0[1], std::vector{
			splines_0[2],
			splines_0[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::cargo_A_turbine_right, 0, 1),
		},
		Globals::cargo_A_shaders, Globals::cargo_A_ubos,
			[](float t, float t_lin, Object &o) {
				// Calculate correct forward from derived func.
				glm::vec3 forw = glm::normalize(util::std2glm(o.curves[0]->eval(t).result()));
				// get vector that points up and is orthogonal to forw.
				glm::vec3 up = util::gs1(forw, util::up);
				// Third vector for complete base.
				glm::vec3 x = glm::normalize(glm::cross(forw, up));

				glm::mat4 rot = {
					   x.x,    x.y,    x.z, 0,
					  up.x,   up.y,   up.z, 0,
					forw.x, forw.y, forw.z, 0,
					     0,      0,      0, 1
				};

				return
					// translate to position.
					glm::translate(util::std2glm(o.pos_curve->eval(t).result())) *
					// rotate model_forw onto forw
					rot * glm::rotate<float>(o.curves[1]->bisect(t_lin).result()[1], glm::vec3(0,0,1));
			}
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::pirate,
		splines_1[0], splines_1[1], std::vector{
			splines_1[2],
			splines_1[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(actions_1[0], actions_1[1], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[2], actions_1[3], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[4], actions_1[5], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[6], actions_1[7], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[8], actions_1[9], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[10], actions_1[11], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[12], actions_1[13], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[14], actions_1[15], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[16], actions_1[17], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[18], actions_1[19], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[20], actions_1[21], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[22], actions_1[23], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[24], actions_1[25], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[26], actions_1[27], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[28], actions_1[29], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[30], actions_1[31], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_1[32], actions_1[33], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_1[34], actions_1[35], Globals::pirate_laser_origin_right, c1),

			std::make_shared<TurbineAction>(Globals::pirate_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::pirate_turbine_right, 0, 1),
		},
		Globals::pirate_shaders, Globals::pirate_ubos
	));

	objs.emplace_back(std::make_unique<Object>(
		Globals::pirate,
		splines_2[0], splines_2[1], std::vector{
			splines_2[2],
			splines_2[3]
		},
		std::vector<std::shared_ptr<ObjectAction>>{
			std::make_shared<LaserAction>(actions_2[0], actions_2[1], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[2], actions_2[3], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[4], actions_2[5], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[6], actions_2[7], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[8], actions_2[9], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[10], actions_2[11], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[12], actions_2[13], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[14], actions_2[15], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[16], actions_2[17], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[18], actions_2[19], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[20], actions_2[21], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[22], actions_2[23], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[24], actions_2[25], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[26], actions_2[27], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[28], actions_2[29], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[30], actions_2[31], Globals::pirate_laser_origin_right, c1),
			std::make_shared<LaserAction>(actions_2[32], actions_2[33], Globals::pirate_laser_origin_left, c1),
			std::make_shared<LaserAction>(actions_2[34], actions_2[35], Globals::pirate_laser_origin_right, c1),

			std::make_shared<TurbineAction>(Globals::pirate_turbine_left, 0, 1),
			std::make_shared<TurbineAction>(Globals::pirate_turbine_right, 0, 1),
		},
		Globals::pirate_shaders, Globals::pirate_ubos
	));

	auto pos_spline = std::make_shared<tinyspline::BSpline>(2, 3, 1);
	pos_spline->setControlPoints({
		0,
		0,
		0,

		20,
		20,
		20
	});

	auto rot_vec = std::make_shared<tinyspline::BSpline>(1, 3, 0);
	rot_vec->setControlPoints(util::glm2std(glm::normalize(glm::vec3{1,.5,1})));

	auto rot_speed = std::make_shared<tinyspline::BSpline>(1, 1, 0);
	rot_speed->setControlPoints({20});

	objs.emplace_back(std::make_unique<Object>(
		std::make_shared<std::vector<geometry>>(std::vector{(*Globals::asteroids)[1]}),
		ast_big_pos[0], ast_big_pos[1], std::vector<std::shared_ptr<tinyspline::BSpline>>{rot_vec, rot_speed},
		std::vector<std::shared_ptr<ObjectAction>>{},
		Globals::asteroid_shaders, Globals::asteroid_ubos, [](float t, float, Object &o) {
			return glm::translate(util::std2glm(o.pos_curve->eval(t).result()))
				* glm::rotate(float(t*o.curves[1]->eval(0).result()[0]), util::std2glm(o.curves[0]->eval(0).result()))
				* glm::scale(glm::vec3(7,7,7));
		}
	));

	auto pos_spline2 = std::make_shared<tinyspline::BSpline>(2, 3, 1);
	pos_spline->setControlPoints({
		0,
		0,
		0,

		20,
		20,
		20
	});

	auto rot_vec2 = std::make_shared<tinyspline::BSpline>(1, 3, 0);
	rot_vec->setControlPoints(util::glm2std(glm::normalize(glm::vec3{1,.5,1})));

	auto rot_speed2 = std::make_shared<tinyspline::BSpline>(1, 1, 0);
	rot_speed->setControlPoints({20});

	objs.emplace_back(std::make_unique<Object>(
		std::make_shared<std::vector<geometry>>(std::vector{(*Globals::asteroids)[2]}),
		ast_big_pos2[0], ast_big_pos2[1], std::vector<std::shared_ptr<tinyspline::BSpline>>{rot_vec2, rot_speed2},
		std::vector<std::shared_ptr<ObjectAction>>{},
		Globals::asteroid_shaders, Globals::asteroid_ubos, [](float t, float, Object &o) {
			return glm::translate(util::std2glm(o.pos_curve->eval(t).result()))
				* glm::rotate(float(t*o.curves[1]->eval(0).result()[0]), util::std2glm(o.curves[0]->eval(0).result()))
				* glm::scale(glm::vec3(6,4,6));
		}
	));

	glm::vec3 zone {300,300,300};
	glm::vec3 center {0,0,0};
	for (size_t i = 0; i != Globals::asteroids->size()*dup_count; ++i) {
		int indx = i/dup_count;
		//// smaller angular distance in center, use root-fn to distribute outside.
		//float len = std::pow(std::rand()/RAND_MAX_F, .58);

		//glm::vec3 p_1 = center + glm::normalize(util::v3_rand())*zone*len;
		//glm::vec3 p_2 = p_1+util::v3_rand()*glm::vec3(200,200,200);

		//auto pos_spline = std::make_shared<tinyspline::BSpline>(2, 3, 1);
		//pos_spline->setControlPoints({
		//	p_1.x,
		//	p_1.y,
		//	p_1.z,
		//	p_2.x,
		//	p_2.y,
		//	p_2.z
		//});
		//auto time_lin = std::make_shared<tinyspline::BSpline>(2, 2, 1);
		//time_lin->setControlPoints({
		//	0,0,
		//	1,1,
		//});

		//auto rot_vec = std::make_shared<tinyspline::BSpline>(1, 3, 0);
		//rot_vec->setControlPoints(util::glm2std(glm::normalize(util::v3_rand())));

		//auto rot_speed = std::make_shared<tinyspline::BSpline>(1, 1, 0);
		//rot_speed->setControlPoints(std::vector<double>{double(std::rand()%10000/100)});

		objs.emplace_back(std::make_unique<Object>(
			std::make_shared<std::vector<geometry>>(std::vector{(*Globals::asteroids)[indx]}),
			asteroid_pos[i], asteroid_time[i], std::vector<std::shared_ptr<tinyspline::BSpline>>{asteroid_axis[i], asteroid_speed[i]},
			std::vector<std::shared_ptr<ObjectAction>>{},
			Globals::asteroid_shaders, Globals::asteroid_ubos, [](float t, float, Object &o) {
				return glm::translate(util::std2glm(o.pos_curve->eval(t).result()))
					*glm::rotate(float(t*o.curves[1]->eval(0).result()[0]), util::std2glm(o.curves[0]->eval(0).result()));
			}
		));
	}

	struct state1 : public ImGuiState {
		int indx_rot;
		int range_rot = 1;
		double rot_offset[2] {0, 0};

		state1(int i_r, int r_r, std::vector<char> render_curves) : ImGuiState{render_curves}, indx_rot{i_r}, range_rot{r_r} { }
	};

	auto asts = std::make_unique<Asteroids>(Globals::asteroids, Globals::asteroids->size(), 300, 900, 400);

	std::vector<char> render_curves(objs.size()+2);
	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines_cam[0], splines_cam[1], splines_0[0], splines_cam[3], splines_cam[4], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &scene) {
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
	}, std::make_unique<state1>(0,3, render_curves), length[0], cam, light_glm, std::move(asts));
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
			Globals::asteroid_shaders, Globals::asteroid_ubos, [](float t, float _, Object &o) {
				return glm::translate(util::std2glm(o.pos_curve->eval(t).result()))
					*glm::rotate(float(t*o.curves[1]->eval(0).result()[0]), util::std2glm(o.curves[0]->eval(0).result()));
			}
		));
	}

	return std::make_unique<Scene>(filename, std::move(objs), Camera{splines[4], splines[5], splines[6], splines[7], splines[8], std::vector<std::shared_ptr<tinyspline::BSpline>>{}}, [](Scene &) { }, std::make_unique<ImGuiState>(std::vector<char>(objs.size()+2)), length[0], cam);
}

void store_scene1(Scene &scene, std::string filename) {
	std::ofstream file;
	int indx = 0;
	for (auto &obj : scene.objects) {
		std::vector<std::shared_ptr<tinyspline::BSpline>> obj_splines;
		obj_splines.push_back(obj->pos_curve);
		obj_splines.push_back(obj->time_curve);
		obj_splines.insert(obj_splines.end(), obj->curves.begin(), obj->curves.end());

		std::vector<float> actions;
		auto action_to_timepoint = [&actions](std::shared_ptr<ObjectAction> &act) {
			actions.push_back(act->from);
			actions.push_back(act->until);
		};
		std::for_each(obj->actions.begin(), obj->actions.end(), action_to_timepoint);

		std::string obj_base_fname = filename + "-" + std::to_string(indx++);
		file.open(obj_base_fname + ".curves");
		util::write_splines(obj_splines, file, '#');
		file.close();

		file.open(obj_base_fname + ".actions");
		util::write_floats(actions, file, '#');
		file.close();
	}

	std::vector<std::shared_ptr<tinyspline::BSpline>> cam_splines;
	cam_splines.push_back(scene.cam.pos_curve);
	cam_splines.push_back(scene.cam.time_pos_curve);
	cam_splines.push_back(scene.cam.look_curve);
	cam_splines.push_back(scene.cam.time_look_curve);
	cam_splines.push_back(scene.cam.zoom_curve);
	cam_splines.insert(cam_splines.end(), scene.cam.curves.begin(), scene.cam.curves.end());

	std::string cam_base_fname = filename + "-cam";
	file.open(cam_base_fname + ".curves");
	util::write_splines(cam_splines, file, '#');
	file.close();

	file.open(filename + ".light");
	std::vector<float> light_std{scene.light_pos.x,scene.light_pos.y,scene.light_pos.z};
	util::write_floats(light_std, file, '#');
	file.close();

	file.open(filename + ".length");
	std::vector<float> len_vec{float(scene.length)};
	util::write_floats(len_vec, file, '#');
	file.close();
}

}
