#include "util.hpp"
#include <functional>
#include <buffer.hpp>
#include <algorithm>
#include <memory>

#define CGAL_EIGEN_3_ENABLED
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/poisson_surface_reconstruction.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/boost/graph/iterator.h>
#include <CGAL/Polygon_mesh_processing/fair.h>
#include <CGAL/Polygon_mesh_processing/refine.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;
typedef CGAL::Point_set_3<Point, Vector> Point_set;
typedef CGAL::Surface_mesh<Point> Mesh;
typedef std::array<std::size_t,3> Facet;
typedef CGAL::Vertex_around_face_iterator<Mesh> vert_iter;
typedef CGAL::Surface_mesh<Point> Mesh;

struct Construct{
	Mesh& mesh;
	Construct(Mesh& mesh)
	  : mesh(mesh) { }
	Construct& operator=(const Facet f)
	{
	  typedef boost::graph_traits<Mesh>::vertex_descriptor vertex_descriptor;
	  typedef boost::graph_traits<Mesh>::vertices_size_type size_type;
	  mesh.add_face(vertex_descriptor(static_cast<size_type>(f[0])),
	                vertex_descriptor(static_cast<size_type>(f[1])),
	                vertex_descriptor(static_cast<size_type>(f[2])));
	  return *this;
	}
	Construct&
	operator*() { return *this; }
	Construct&
	operator++() { return *this; }
	Construct
	operator++(int) { return *this; }
};

glm::vec3 rand_max {RAND_MAX, RAND_MAX, RAND_MAX};

namespace util {
	glm::vec3 std2glm(std::vector<double> vec) {
		return glm::vec3(vec[0], vec[1], vec[2]);
	}

	std::vector<double> glm2std(glm::vec3 v) {
		return {v.x, v.y, v.z};
	}

	std::vector<geometry> load_scene_full_mesh(const char* filename, bool smooth) {
		Assimp::Importer importer;
		int process = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices;
		if (smooth) {
			process |= aiProcess_GenSmoothNormals;
		} else {
			process |= aiProcess_GenNormals;
		}
		const aiScene* scene = importer.ReadFile(SHADER_ROOT + "../models/" + filename, process);
		if (scene == nullptr) return {};

		std::vector<glm::vec4> colors;
		for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
			aiMaterial* mat = scene->mMaterials[i];
			aiColor3D color(1.f, 1.f, 1.f);
			mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			colors.push_back(glm::vec4(color[0], color[1], color[2], 1.f));
		}

		std::vector<geometry> objects;
		std::function<void(aiNode*, glm::mat4)> traverse;
		traverse = [&](aiNode* node, glm::mat4 t) {
			aiMatrix4x4 aim = node->mTransformation;
			glm::mat4 new_t(
				aim.a1, aim.b1, aim.c1, aim.d1,
				aim.a2, aim.b2, aim.c2, aim.d2,
				aim.a3, aim.b3, aim.c3, aim.d3,
				aim.a4, aim.b4, aim.c4, aim.d4
			);
			t = new_t * t;

			aiMatrix4x4 local_t;
			if (node->mNumMeshes > 0) {
				uint32_t total_vertices = 0;
				uint32_t total_faces = 0;
				for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					total_vertices += mesh->mNumVertices;
					total_faces += mesh->mNumFaces;
				}

				geometry m{};
				m.positions.resize(total_vertices);
				m.normals.resize(total_vertices);
				m.colors.resize(total_vertices);
				m.faces.resize(total_faces);

				float* vbo_data = new float[total_vertices * 10];
				unsigned int* ibo_data = new unsigned int[total_faces * 3];

				uint32_t vert_base_indx = 0;
				uint32_t face_base_indx = 0;
				for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

					for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
						glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
						glm::vec3 nrm(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
						glm::vec4 col = m.colors[i];
						if (mesh->HasVertexColors(0)) {
							col[0] = mesh->mColors[0][i].r;
							col[1] = mesh->mColors[0][i].g;
							col[2] = mesh->mColors[0][i].b;
							col[3] = mesh->mColors[0][i].a;
						} else {
							col = colors[mesh->mMaterialIndex];
						}

						m.positions[vert_base_indx + i] = pos;
						m.normals[vert_base_indx + i] = nrm;
						m.colors[vert_base_indx + i] = col;

						vbo_data[(vert_base_indx + i) * 10 + 0] = pos[0];
						vbo_data[(vert_base_indx + i) * 10 + 1] = pos[1];
						vbo_data[(vert_base_indx + i) * 10 + 2] = pos[2];
						vbo_data[(vert_base_indx + i) * 10 + 3] = nrm[0];
						vbo_data[(vert_base_indx + i) * 10 + 4] = nrm[1];
						vbo_data[(vert_base_indx + i) * 10 + 5] = nrm[2];
						vbo_data[(vert_base_indx + i) * 10 + 6] = col[0];
						vbo_data[(vert_base_indx + i) * 10 + 7] = col[1];
						vbo_data[(vert_base_indx + i) * 10 + 8] = col[2];
						vbo_data[(vert_base_indx + i) * 10 + 9] = col[3];
					}

					for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
						glm::uvec3 face(mesh->mFaces[i].mIndices[0],
										mesh->mFaces[i].mIndices[1],
										mesh->mFaces[i].mIndices[2]);
						m.faces[i] = face;
						ibo_data[(face_base_indx + i) * 3 + 0] = vert_base_indx + face[0];
						ibo_data[(face_base_indx + i) * 3 + 1] = vert_base_indx + face[1];
						ibo_data[(face_base_indx + i) * 3 + 2] = vert_base_indx + face[2];
					}
					vert_base_indx += mesh->mNumVertices;
					face_base_indx += mesh->mNumFaces;
				}

				glGenVertexArrays(1, &m.vao);
				glBindVertexArray(m.vao);
				m.vbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, total_vertices * 10 * sizeof(float), vbo_data);
				m.ibo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, total_faces * 3 * sizeof(unsigned int), ibo_data);
				glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3*sizeof(float)));
				glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6*sizeof(float)));
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
				m.transform = t;
				m.vertex_count = 3*total_faces;
				objects.push_back(m);

				delete [] vbo_data;
				delete [] ibo_data;
			}

			for (uint32_t i = 0; i < node->mNumChildren; ++i) {
				traverse(node->mChildren[i], t);
			}
		};

		traverse(scene->mRootNode, glm::identity<glm::mat4>());
		return objects;
	}

	glm::vec3 v3_rand() {
		return glm::vec3(std::rand(), std::rand(), std::rand())/rand_max;
	}

	std::shared_ptr<geometry> create_asteroid() {
		Mesh mesh;
		std::vector<Point> points;
		CGAL::Random_points_on_sphere_3<Point> gen(1);

		float col[4] = {.5,.5,.5};
	
		const size_t count = 40; 
		float v[count*10];
		points.reserve(count);

		for (size_t i = 0; i != count; ++i, ++gen) {
			// normal = position.
			mesh.add_vertex(*gen);
		}

		mesh.reserve(count, 0, 0);
		Construct c{mesh};
		CGAL::advancing_front_surface_reconstruction(
			mesh.points().begin(),
			mesh.points().end(),
			c
		);

		for (auto it{mesh.points().begin()}; it != mesh.points().end(); ++it) {
			//values between 0.6, 1.4.
			float val = (std::rand() % 400) / 1000.f+.9f;
			*it = it->transform(CGAL::Aff_transformation_3<Kernel>(CGAL::SCALING, val));
		}

		auto p_it = mesh.points().begin();
		for (int i = 0; i != count; ++i, ++p_it) {
			v[10 * i + 0] = p_it->x();
			v[10 * i + 1] = p_it->y();
			v[10 * i + 2] = p_it->z();
			v[10 * i + 3] = p_it->x();
			v[10 * i + 4] = p_it->y();
			v[10 * i + 5] = p_it->z();
			v[10 * i + 6] = col[0];
			v[10 * i + 7] = col[1];
			v[10 * i + 8] = col[2];
			v[10 * i + 9] = col[3];
		}

		auto i = std::make_unique<unsigned int[]>(mesh.faces().size()*3);
		unsigned int *it = &i[0];
		std::for_each(mesh.faces().begin(), mesh.faces().end(), [&mesh, &it](Mesh::Face_index &face){
			vert_iter face_it {mesh.halfedge(face), mesh};
			*(it++) = *(face_it++);
			*(it++) = *(face_it++);
			*(it++) = *(face_it++);
		});


		auto m = std::make_shared<geometry>();
        glGenVertexArrays(1, &(m->vao));
        glBindVertexArray(m->vao);
        m->vbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, count * 10 * sizeof(float), v);
        m->ibo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, mesh.faces().size() * 3 * sizeof(unsigned int), i.get());
        glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3*sizeof(float)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);

		m->transform = glm::identity<glm::mat4>();
		m->vertex_count = mesh.faces().size()*3;

        return m;
	}

	glm::vec3 gs1(glm::vec3 a, glm::vec3 b) {
		return glm::normalize(b - glm::dot(a, b)*a);
	}

	void plot_spline(
		const tinyspline::BSpline &spline,
		std::string name,
		std::function<float(const tinyspline::BSpline &spline, float t)> eval_func) {

		const size_t count = 200;
		float plot[count];
		for (size_t i = 0; i != count; ++i)
			plot[i] = eval_func(spline, float(i)/count);
		
		ImGui::PlotLines(
			name.c_str(), plot, count, 0, 
			NULL, FLT_MAX, FLT_MAX, 
			ImVec2(280, 100));
	}

	void control_point_edit(std::shared_ptr<tinyspline::BSpline> *spline, int *indx, int *range, double *offset) {
		int dim = (*spline)->dimension();
		ImGui::SliderInt("point", indx, 0, (*spline)->numControlPoints()-1);
		if (ImGui::Button("Add")) {
			std::vector<double> ctrl = (*spline)->controlPoints();
			// Copy controlpoint at indx.
			std::vector<double> point{};
			point.insert(point.begin(), ctrl.begin()+*indx*dim, ctrl.begin()+(*indx+1)*dim);
			ctrl.insert(ctrl.begin()+*indx*dim, point.begin(), point.end());
			unsigned int deg = (*spline)->degree();
			*spline = std::make_shared<tinyspline::BSpline>((*spline)->numControlPoints()+1, dim, deg < 3 ? deg+1 : deg);
			(*spline)->setControlPoints(ctrl);
		}

		std::vector<tinyspline::real> ctrl_point = (*spline)->controlPointAt(*indx);

		if (ImGui::InputInt("range", range))
			for (int i = 0; i != dim; ++i)
				offset[i] = ctrl_point[i];

		// I guess it's fine if these get destroyed soon??
		auto mins = std::make_unique<double[]>(3);
		auto maxs = std::make_unique<double[]>(3);
		for (int i = 0; i != dim; ++i) {
			mins[i] = offset[i]-*range;
			maxs[i] = offset[i]+*range;
			ImGui::SliderScalar(std::to_string(i).c_str(), ImGuiDataType_Double, &ctrl_point[i], &mins[i], &maxs[i]);	
		}
        (*spline)->setControlPointAt(*indx, ctrl_point);
	}

	float eval_timespline(const tinyspline::BSpline &spline, float t) {
		return std::clamp<float>(spline.eval(t).result()[0], 0, 1);
	}

	int getTimeDelta(std::chrono::time_point<std::chrono::system_clock> start_time) {
		auto now = std::chrono::system_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now-start_time).count();
	}

	std::vector<std::shared_ptr<tinyspline::BSpline>> read_splines(std::istream &stream, char delim) {
		std::vector<std::shared_ptr<tinyspline::BSpline>> splines {};
		std::string temp;
		while (std::getline(stream, temp, delim)) {
			auto spl = std::make_shared<tinyspline::BSpline>(tinyspline::BSpline::parseJson(temp));
			splines.push_back(spl);
		}

		return splines;
	}

	void write_splines(std::vector<std::shared_ptr<tinyspline::BSpline>> &vec, std::ostream &stream, char delim) {
		for (auto spline : vec)
			stream << spline->toJson() << delim;
	}

	void action_edit(std::vector<std::shared_ptr<ObjectAction>>& actions) {
		for (unsigned int i = 0; i != actions.size(); ++i)
			ImGui::SliderFloat(std::to_string(i).c_str(), &actions[i]->start_time, 0, 1);
	}

	void edit_boolvec(std::vector<char> &vec) {
		for (unsigned int i = 0; i != vec.size(); ++i)
			ImGui::Checkbox(std::to_string(i).c_str(), (bool *) &vec[i]);
	}

	void write_floats(std::vector<float> &vec, std::ostream &str, char delim) {
		for (float f : vec)
			 str << f << delim;
	}

	std::vector<float> read_floats(std::istream &str, char delim) {
		std::vector<float> floats{};
		std::string temp;
		while (std::getline(str, temp, delim)) {
			floats.push_back(std::stof(temp));
		}

		return floats;
	}
}
