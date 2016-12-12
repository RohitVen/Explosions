#include <GL/glew.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <dirent.h>

#include "bone_geometry.h"
#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
#include "gui.h"
#include "../lib/jpegio.h"
#include "../lib/image.h"
#include "../lib/lodepng.h"
#include "ParticleSystem.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>
#include <cmath>

int window_width = 800, window_height = 600;
const std::string window_title = "Skinning";

const char* vertex_shader =
#include "shaders/default.vert"
;

const char* geometry_shader =
#include "shaders/default.geom"
;

const char* fragment_shader =
#include "shaders/default.frag"
;

const char* floor_fragment_shader =
#include "shaders/floor.frag"
;

const char* bill_vertex_shader = 
#include "shaders/bill.vert"
;

const char* bill_fragment_shader = 
#include "shaders/bill.frag"
;

const char* tex_fragment_shader = 
#include "shaders/tex.frag"
;

// FIXME: Add more shaders here.

void ErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << "\n";
}

GLFWwindow* init_glefw()
{
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}
	glfwSetErrorCallback(ErrorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	auto ret = glfwCreateWindow(window_width, window_height, window_title.data(), nullptr, nullptr);
	CHECK_SUCCESS(ret != nullptr);
	glfwMakeContextCurrent(ret);
	glewExperimental = GL_TRUE;
	CHECK_SUCCESS(glewInit() == GLEW_OK);
	glGetError();  // clear GLEW's error for it
	glfwSwapInterval(1);
	const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);    // version as a string
	std::cout << "Renderer: " << renderer << "\n";
	std::cout << "OpenGL version supported:" << version << "\n";

	return ret;
}

unsigned compileShader(const char* source_ptr, int type)
{
	if (!source_ptr)
		return 0;
	GLuint ret = 0;
	CHECK_GL_ERROR(ret = glCreateShader(type));
#if 0
	std::cerr << __func__ << " shader id " << ret << " type " << type << "\tsource:\n" << source_ptr << std::endl;
#endif
	CHECK_GL_ERROR(glShaderSource(ret, 1, &source_ptr, nullptr));
	glCompileShader(ret);
	CHECK_GL_SHADER_ERROR(ret);
	return ret;
}

void bind_uniforms(std::vector<ShaderUniform>& uniforms,
		const std::vector<unsigned>& unilocs)
{
	for (size_t i = 0; i < uniforms.size(); i++) {
		const auto& uni = uniforms[i];
		//std::cerr << "binding " << uni.name << std::endl;
		CHECK_GL_ERROR(uni.binder(unilocs[i], uni.data_source()));
	}
}

struct particle
{
	ParticleBillboard p;
	float dist;

	particle(ParticleBillboard pb, float d)
	{
		p = pb;
		dist = d;
	}

	bool operator< (const particle &part) const
	{

		return dist > part.dist;
	}
};


int main(int argc, char* argv[])
{
	ParticleSystem ps;
	string path = "/v/filer4b/v38q001/rohitven/Desktop/CS354/A4/explosions/src/json/psdefault.json";
	ps.ConfigDefault(path);

	GLFWwindow *window = init_glefw();
	GUI gui(window);

	std::vector<glm::vec4> floor_vertices;
	std::vector<glm::uvec3> floor_faces;
	create_floor(floor_vertices, floor_faces);

	// FIXME: add code to create bone and cylinder geometry

	Mesh mesh;

	/*
	 * GUI object needs the mesh object for bone manipulation.
	 */
	gui.assignMesh(&mesh);

	glm::vec4 light_position = glm::vec4(0.0f, 100.0f, 0.0f, 1.0f);
	MatrixPointers mats; // Define MatrixPointers here for lambda to capture
	/*
	 * In the following we are going to define several lambda functions to bind Uniforms.
	 * 
	 * Introduction about lambda functions:
	 *      http://en.cppreference.com/w/cpp/language/lambda
	 *      http://www.stroustrup.com/C++11FAQ.html#lambda
	 */
	auto matrix_binder = [](int loc, const void* data) {
		glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)data);
	};
	auto bone_matrix_binder = [&mesh](int loc, const void* data) {
		auto nelem = mesh.getNumberOfBones();
		glUniformMatrix4fv(loc, nelem, GL_FALSE, (const GLfloat*)data);
	};
	auto vector_binder = [](int loc, const void* data) {
		glUniform4fv(loc, 1, (const GLfloat*)data);
	};
	auto vector3_binder = [](int loc, const void* data) {
		glUniform3fv(loc, 1, (const GLfloat*)data);
	};
	auto float_binder = [](int loc, const void* data) {
		glUniform1fv(loc, 1, (const GLfloat*)data);
	};
	/*
	 * These lambda functions below are used to retrieve data
	 */
	auto std_model_data = [&mats]() -> const void* {
		return mats.model;
	}; // This returns point to model matrix
	glm::mat4 floor_model_matrix = glm::mat4(1.0f);
	auto floor_model_data = [&floor_model_matrix]() -> const void* {
		return &floor_model_matrix[0][0];
	}; // This return model matrix for the floor.
	auto std_view_data = [&mats]() -> const void* {
		return mats.view;
	};
	auto std_camera_data  = [&gui]() -> const void* {
		return &gui.getCamera()[0];
	};
	auto std_proj_data = [&mats]() -> const void* {
		return mats.projection;
	};
	auto std_light_data = [&light_position]() -> const void* {
		return &light_position[0];
	};
	auto alpha_data  = [&gui]() -> const void* {
		static const float transparet = 0.5; // Alpha constant goes here
		static const float non_transparet = 1.0;
		if (gui.isTransparent())
			return &transparet;
		else
			return &non_transparet;
	};
	// FIXME: add more lambdas for data_source if you want to use RenderPass.
	//        Otherwise, do whatever you like here
	ShaderUniform std_model = { "model", matrix_binder, std_model_data };
	ShaderUniform floor_model = { "model", matrix_binder, floor_model_data };
	ShaderUniform std_view = { "view", matrix_binder, std_view_data };
	ShaderUniform std_camera = { "camera_position", vector3_binder, std_camera_data };
	ShaderUniform std_proj = { "projection", matrix_binder, std_proj_data };
	ShaderUniform std_light = { "light_position", vector_binder, std_light_data };
	ShaderUniform object_alpha = { "alpha", float_binder, alpha_data };
	// FIXME: define more ShaderUniforms for RenderPass if you want to use it.
	//        Otherwise, do whatever you like here

	std::vector<glm::vec2>& uv_coordinates = mesh.uv_coordinates;
	RenderDataInput object_pass_input;
	object_pass_input.assign(0, "vertex_position", nullptr, mesh.vertices.size(), 4, GL_FLOAT);
	object_pass_input.assign(1, "normal", mesh.vertex_normals.data(), mesh.vertex_normals.size(), 4, GL_FLOAT);
	object_pass_input.assign(2, "uv", uv_coordinates.data(), uv_coordinates.size(), 2, GL_FLOAT);
	object_pass_input.assign_index(mesh.faces.data(), mesh.faces.size(), 3);
	object_pass_input.useMaterials(mesh.materials);
	RenderPass object_pass(-1,
			object_pass_input,
			{
			  vertex_shader,
			  geometry_shader,
			  fragment_shader
			},
			{ std_model, std_view, std_proj,
			  std_light,
			  std_camera, object_alpha },
			{ "fragment_color" }
			);

	RenderDataInput floor_pass_input;
	floor_pass_input.assign(0, "vertex_position", floor_vertices.data(), floor_vertices.size(), 4, GL_FLOAT);	
	floor_pass_input.assign_index(floor_faces.data(), floor_faces.size(), 3);
	RenderPass floor_pass(-1,
			floor_pass_input,
			{ vertex_shader, geometry_shader, floor_fragment_shader},
			{ floor_model, std_view, std_proj, std_light },
			{ "fragment_color" }
			);

	// FIXME: Create the RenderPass objects for bones here.
	//        Otherwise do whatever you like.

	//Start Billboard Code//

	std::vector<glm::vec4> bill_vertices;
	std::vector<glm::uvec3> bill_faces;
	std::vector<glm::vec4> bill_center;
	std::vector<glm::vec2> bill_uv;
	std::vector<float> rot;
	std::vector<float> scale;
	std::vector<glm::vec4> colors;
	std::vector<float> alpha;
	std::vector<particle> particles;
	double toRad = M_PI/180;

	//End Billboard Code//

	//End Texture Code//

	bool draw_floor = true;
	bool draw_skeleton = true;
	bool draw_object = true;
	bool draw_cylinder = true;
	bool draw_bill = true;

	while (!glfwWindowShouldClose(window)) {
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &window_width, &window_height);
		glViewport(0, 0, window_width, window_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);

		gui.updateMatrices();
		mats = gui.getMatrixPointers();

		int current_bone = gui.getCurrentBone();
#if 1
		draw_cylinder = (current_bone != -1 && gui.isTransparent());	
#else
		draw_cylinder = true;
#endif
		// FIXME: Draw bones first.

		// Then draw floor.
		if (draw_floor) {	
			floor_pass.setup();
			// Draw our triangles.
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, floor_faces.size() * 3, GL_UNSIGNED_INT, 0));		
		}

		if(gui.kickOff)
		{
			ps.Update();
			glm::vec3 eye = gui.eye_;
			particles.clear();
			for(int i = 0; i < ps.active_billboards.size(); i++)
			{
				ParticleBillboard b = ps.active_billboards[i];
				glm::vec3 u = b.position;
				particle p (b, glm::distance(u, eye));
				particles.push_back(p);
			}
			std::sort(&particles[0], &particles[particles.size()]);
			bill_center.clear();
			bill_faces.clear();
			bill_uv.clear();
			scale.clear();
			rot.clear();
			colors.clear();
			alpha.clear();
			for(int i = 0; i < particles.size(); i++)
			{
				ParticleBillboard b = particles[i].p;
				bill_center.push_back(glm::vec4(b.position,1));
				scale.push_back(b.scale);
				rot.push_back(b.rotation * toRad);
				alpha.push_back(b.alpha);
				alpha.push_back(b.alpha);
				alpha.push_back(b.alpha);
				alpha.push_back(b.alpha);
				colors.push_back(glm::vec4(b.color, 1));
				colors.push_back(glm::vec4(b.color, 1));
				colors.push_back(glm::vec4(b.color, 1));
				colors.push_back(glm::vec4(b.color, 1));
			}
		}

		bill_vertices.clear();
		create_bill(&gui, bill_vertices, bill_faces, bill_center, scale, rot);

		if(draw_bill)
		{
	        RenderDataInput bill_pass_input;
			bill_pass_input.assign(0, "position", bill_vertices.data(), bill_vertices.size(), 4, GL_FLOAT);	
			bill_pass_input.assign(1, "color", colors.data(), colors.size(), 4, GL_FLOAT);	
			bill_pass_input.assign(2, "alpha", alpha.data(), alpha.size(), 1, GL_FLOAT);	
			bill_pass_input.assign_index(bill_faces.data(), bill_faces.size(), 3);
			RenderPass bill_pass(-1,
					bill_pass_input,
					{ bill_vertex_shader, nullptr, bill_fragment_shader},
					{ std_view, std_proj },
					{ "color" }
					);

			bill_pass.setup();

			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, bill_faces.size() * 3, GL_UNSIGNED_INT, 0));
		}
		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
#if 0
	for (size_t i = 0; i < images.size(); ++i)
		delete [] images[i].bytes;
#endif
	exit(EXIT_SUCCESS);
}
