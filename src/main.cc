#include "../../soil/src/SOIL.h"
#include <GL/glew.h>
#include <dirent.h>

#include "bone_geometry.h"
#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
#include "gui.h"
#include "pngimage.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>
#include <iostream>



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

// DONE
const char* bone_vertex_shader =
R"zzz(
#version 330 core
uniform mat4 projection;
// uniform mat4 model;
uniform mat4 view;
in vec4 position;

void main()
{
	gl_Position = projection * view * position;
}
)zzz";

const char* bone_fragment_shader =
R"zzz(#version 330 core
out vec4 color;

void main()
{
	color = vec4(255.0, 204.0, 0.0, 1.0);
}
)zzz";

const char* cyl_fragment_shader =
R"zzz(#version 330 core
out vec4 color;

void main()
{
	color = vec4(0.0, 191.0, 255.0, 1.0);
}
)zzz";

const char* bill_vertex_shader =
R"zzz(
#version 330 core
uniform mat4 projection;
// uniform mat4 model;
uniform mat4 view;
in vec4 position;

void main()
{
	vec4 center = vec4(0,0,0,1);
	center = projection * view * center;
	vec4 cam_up = vec4(view[0][1], view[1][1], view[2][1], view[3][1]);
	vec4 cam_r = vec4(view[0][0], view[1][0], view[2][0], view[3][0]);
	vec4 p = center + cam_r * position.x * 100 + cam_up * position.y * 100;
	gl_Position = projection * view * p;
}
)zzz";

const char* tex_vertex_shader =
R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 view;
in vec4 position;
in vec2 uv;
out vec2 uv_coords;

void main()
{
	vec4 center = vec4(0,0,0,1);
	center = projection * view * center;
	vec4 cam_up = vec4(view[0][1], view[1][1], view[2][1], view[3][1]);
	vec4 cam_r = vec4(view[0][0], view[1][0], view[2][0], view[3][0]);
	vec4 p = center + cam_r * position.x * 100 + cam_up * position.y * 100;
	gl_Position = projection * view * p;
	uv_coords = uv;
}
)zzz";

const char* tex_fragment_shader =
R"zzz(#version 330 core
in vec2 uv_coords;
out vec4 color;
uniform sampler2D textureSampler;

void main()
{
	color = texture(textureSampler, uv_coords);
}
)zzz";

void ErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << "\n";
}

GLFWwindow* init_glefw()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);
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

GLuint loadPNG( std::string filename, std::vector<unsigned char> &data ) 
{
	int start = (int) filename.find_last_of('.');
	int end = (int) filename.size() - 1;
	if (start >= 0 && start < end) {
		std::string ext = filename.substr(start, end);
		if (!ext.compare(".png")) {
			png_cleanup(1);
			std::cout<<"\npng_init: "<<png_init(filename.c_str(), window_width, window_height);
			if (!png_init(filename.c_str(), window_width, window_height)) {
				double gamma = 2.2;
				int channels, rowBytes;
				unsigned char* indata = png_get_image(gamma, channels, rowBytes);
				int bufsize = rowBytes * window_height;
				// data = new unsigned char[bufsize];
				for (int j = 0; j < window_height; j++)
					for (int i = 0; i < rowBytes; i += channels)
						for (int k = 0; k < channels; k++)
							data.push_back(*(indata + k + i + (window_height - j - 1) * rowBytes));
				png_cleanup(1);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	// if (argc < 2) {
	// 	std::cerr << "Input model file is missing" << std::endl;
	// 	std::cerr << "Usage: " << argv[0] << " <PMD file>" << std::endl;
	// 	return -1;
	// }
	GLFWwindow *window = init_glefw();
	GUI gui(window);

	std::vector<glm::vec4> floor_vertices;
	std::vector<glm::uvec3> floor_faces;
	create_floor(floor_vertices, floor_faces);

	// FIXME: add code to create bone and cylinder geometry

	Mesh mesh;
	// mesh.loadpmd(argv[1]);
	std::cout << "Loaded object  with  " << mesh.vertices.size()
		<< " vertices and " << mesh.faces.size() << " faces.\n";

	glm::vec4 mesh_center = glm::vec4(0.0f);
	// for (size_t i = 0; i < mesh.vertices.size(); ++i) {
	// 	mesh_center += mesh.vertices[i];
	// }
	// mesh_center /= mesh.vertices.size();


	/*
	 * GUI object needs the mesh object for bone manipulation.
	 */
	gui.assignMesh(&mesh);

	std::vector<glm::vec4> bone_vertices;
	std::vector<glm::uvec2> bone_faces;
	create_bones(bone_vertices, bone_faces, mesh); //Get the vertices of the bones!

	std::vector<glm::vec4> cyl_vertices;
	std::vector<glm::uvec2> cyl_faces;
	

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

	// FIXME: Create the RenderPass objects for bones here.
	//        Otherwise do whatever you like.


	//Start Billboard Code//

	std::vector<glm::vec4> bill_vertices;
	std::vector<glm::uvec3> bill_faces;
	create_bill(bill_vertices, bill_faces);

	RenderDataInput bill_pass_input;
	bill_pass_input.assign(0, "vertex_position", bill_vertices.data(), bill_vertices.size(), 4, GL_FLOAT);
	bill_pass_input.assign_index(bill_faces.data(), bill_faces.size(), 3);
	RenderPass bill_pass(-1,
			bill_pass_input,
			{ bill_vertex_shader, nullptr, cyl_fragment_shader},
			{ std_model, std_view, std_proj},
			{ "color" }
			);

	//End Billboard Code//



	//Start Texture Code//
	GLuint vao;
	GLuint vbo;
	glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {
    //  Position      UV coords
    -0.5f,  0.5f,    0.0f, 0.0f, // Top-left
     0.5f,  0.5f,    1.0f, 0.0f, // Top-right
     0.5f, -0.5f,    1.0f, 1.0f, // Bottom-right
    -0.5f, -0.5f,    0.0f, 1.0f  // Bottom-left
    };


    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create an element array
    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &tex_vertex_shader, NULL);
    glCompileShader(vertexShader);

     // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &tex_fragment_shader, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "color");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

    GLint texAttrib = glGetAttribLocation(shaderProgram, "uv");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

     // Load textures
    GLuint textures[1];
    glGenTextures(1, textures);

    int width, height;
    unsigned char* data;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    data = SOIL_load_image("/u/rohitven/Desktop/CS354/A4/explosions/assets/cloud_smoke_textures/Cloud and Smoke Textures/cloud_14.png", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    SOIL_free_image_data(data);

    glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);






	// std::string filename = "/u/rohitven/Desktop/CS354/A4/explosions/assets/cloud_smoke_textures/Cloud and Smoke Textures/cloud_14.png";
	// std::vector<unsigned char> data;
	// GLuint texture = loadPNG(filename, data); 

	// std::vector<glm::vec2> tex_uv;

	// tex_uv.push_back(glm::vec2(0,0));
	// tex_uv.push_back(glm::vec2(1,0));
	// tex_uv.push_back(glm::vec2(1,1));
	// tex_uv.push_back(glm::vec2(1,1));
	// tex_uv.push_back(glm::vec2(0,1));
	// tex_uv.push_back(glm::vec2(0,0));

	// GLuint tex;
	// CHECK_GL_ERROR(glGenTextures(1, &tex));
	// glBindTexture(GL_TEXTURE_2D, tex);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glGenerateMipmap(GL_TEXTURE_2D);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RBG, GL_UNSIGNED_BYTE, data);
	// float pixels[] = {
 //    0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
 //    1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
	// };
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);



	// RenderDataInput tex_pass_input;
	// 	tex_pass_input.assign(0, "vertex_position", bill_vertices.data(), bill_vertices.size(), 4, GL_FLOAT);
	// 	tex_pass_input.assign(1, "uv", tex_uv.data(), tex_uv.size(), 2, GL_FLOAT);
	// 	tex_pass_input.assign_index(bill_faces.data(), bill_faces.size(), 3);
	// 	RenderPass tex_pass(-1,
	// 			tex_pass_input,
	// 			{ tex_vertex_shader, nullptr, fragment_shader},
	// 			{ std_model, std_view, std_proj},
	// 			{ "fragment color" }
	// 			);
	//End Texture Code//





	// RenderDataInput bone_pass_input;
	// bone_pass_input.assign(0, "vertex_postion", bone_vertices.data(), bone_vertices.size(), 4, GL_FLOAT);
	// bone_pass_input.assign_index(bone_faces.data(), bone_faces.size(), 2);
	// RenderPass bone_pass(-1, bone_pass_input, 
	// 	{bone_vertex_shader, nullptr, bone_fragment_shader}, 
	// 	{ std_view, std_proj}, 
	// 	{ "color" }
	// 	);

	// RenderDataInput cyl_pass_input;
	// cyl_pass_input.assign(0, "vertex_postion", cyl_vertices.data(), cyl_vertices.size(), 4, GL_FLOAT);
	// cyl_pass_input.assign_index(cyl_faces.data(), cyl_faces.size(), 2);
	// RenderPass cyl_pass(-1, cyl_pass_input, 
	// 	{bone_vertex_shader, nullptr, cyl_fragment_shader}, 
	// 	{std_model, std_view, std_proj}, 
	// 	{ "color" }
	// 	);

	RenderDataInput floor_pass_input;
	floor_pass_input.assign(0, "vertex_position", floor_vertices.data(), floor_vertices.size(), 4, GL_FLOAT);
	floor_pass_input.assign_index(floor_faces.data(), floor_faces.size(), 3);
	RenderPass floor_pass(-1,
			floor_pass_input,
			{ vertex_shader, geometry_shader, floor_fragment_shader},
			{ floor_model, std_view, std_proj, std_light },
			{ "fragment_color" }
			);
	float aspect = 0.0f;
	std::cout << "center = " << mesh.getCenter() << "\n";

	bool draw_floor = true;
	bool draw_skeleton = true;
	bool draw_object = true;
	bool draw_cylinder = true;
	bool draw_bone = gui.isTransparent();
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
		draw_bone = gui.isTransparent();

		if(draw_bill)
		{
			// std::cout<<"\nDraw the bill!";
			bill_pass.setup();
			// tex_pass.setup();
			// CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, tex_uv.size() * 2, GL_UNSIGNED_INT, 0));
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, bill_faces.size() * 3, GL_UNSIGNED_INT, 0));
		}

		// if(draw_bone)
		// {
		// 	bone_pass.setup();
		// 	CHECK_GL_ERROR(glDrawElements(GL_LINES, bone_faces.size() * 3, GL_UNSIGNED_INT, 0));
		// }

		// if(draw_cylinder)
		// {
		// 	cyl_vertices.clear();
		// 	cyl_faces.clear();
		// 	create_cylinder(cyl_vertices, cyl_faces, mesh, current_bone); //Create cylinders!!
		// 	cyl_pass_input.assign(0, "vertex_postion", cyl_vertices.data(), cyl_vertices.size(), 4, GL_FLOAT);
		// 	cyl_pass_input.assign_index(cyl_faces.data(), cyl_faces.size(), 2);
		// 	RenderPass cyl_pass(-1, cyl_pass_input, 
		// 		{bone_vertex_shader, nullptr, cyl_fragment_shader}, 
		// 		{std_model, std_view, std_proj}, 
		// 		{ "color" }
		// 		);
		// 	cyl_pass.setup();
		// 	CHECK_GL_ERROR(glDrawElements(GL_LINES, cyl_faces.size() * 3, GL_UNSIGNED_INT, 0));
		// }


		if (draw_floor) {
			floor_pass.setup();
			// Draw our triangles.
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, floor_faces.size() * 3, GL_UNSIGNED_INT, 0));
		}
		if (draw_object) {
			if (gui.isPoseDirty()) {
				mesh.updateAnimation();
				object_pass.updateVBO(0,
						      mesh.animated_vertices.data(),
						      mesh.animated_vertices.size());
#if 0
				// For debugging if you need it.
				for (int i = 0; i < 4; i++) {
					std::cerr << " Vertex " << i << " from " << mesh.vertices[i] << " to " << mesh.animated_vertices[i] << std::endl;
				}
#endif
				gui.clearPose();
			}
			object_pass.setup();
			int mid = 0;
			while (object_pass.renderWithMaterial(mid))
				mid++;
#if 0	
			// For debugging also
			if (mid == 0) // Fallback
				CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, mesh.faces.size() * 3, GL_UNSIGNED_INT, 0));
#endif
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
