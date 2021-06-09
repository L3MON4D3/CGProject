#include "util.hpp"
#include <functional>
#include <buffer.hpp>

namespace util {
	glm::vec3 std2glm(std::vector<double> vec) {
		return glm::vec3(vec[0], vec[1], vec[2]);
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

	glm::vec3 gs1(glm::vec3 a, glm::vec3 b) {
		return glm::normalize(b - glm::normalize(glm::dot(a, b)*a));
	}
}
