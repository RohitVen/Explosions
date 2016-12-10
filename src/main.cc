#include <GL/glew.h>
#include <dirent.h>

#include "bone_geometry.h"
#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
#include "gui.h"
#include "../lib/jpegio.h"
#include "../lib/image.h"
#include "shader.h"

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

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cerr << "Input model file is missing" << std::endl;
		std::cerr << "Usage: " << argv[0] << " <PMD file>" << std::endl;
		return -1;
	}
	GLFWwindow *window = init_glefw();
	GUI gui(window);

	std::vector<glm::vec4> floor_vertices;
	std::vector<glm::uvec3> floor_faces;
	create_floor(floor_vertices, floor_faces);

	// FIXME: add code to create bone and cylinder geometry

	Mesh mesh;
	mesh.loadpmd(argv[1]);
	std::cout << "Loaded object  with  " << mesh.vertices.size()
		<< " vertices and " << mesh.faces.size() << " faces.\n";

	glm::vec4 mesh_center = glm::vec4(0.0f);
	for (size_t i = 0; i < mesh.vertices.size(); ++i) {
		mesh_center += mesh.vertices[i];
	}
	mesh_center /= mesh.vertices.size();

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

	// FIXME: Create the RenderPass objects for bones here.
	//        Otherwise do whatever you like.

	//Start Billboard Code//

	std::vector<glm::vec4> bill_vertices;
	std::vector<glm::uvec3> bill_faces;
	std::vector<glm::vec4> bill_center;
	std::vector<glm::mat4> transforms;
	glm::vec3 eye = gui.getCamera();
	double radius = 0.75;
	double toRad = M_PI/180;
	double deg = 0;
	bill_center.push_back(glm::vec4(0,2.5,0,1));
	// while(deg < 360)
	// {
	// 	double rad = deg * toRad;
	// 	bill_center.push_back((glm::vec4(radius*cos(rad), 2.5+radius*sin(rad), 0, 1)));
	// 	deg += 45;
	// 	if(deg < 360)
	// 	{
	// 		rad = deg * toRad;
	// 		bill_center.push_back((glm::vec4(radius*cos(rad), 2.5+radius*sin(rad), 0, 1)));
	// 	}
		
	// }
	// bill_center.push_back(glm::vec4(0,3,0,1));
	create_bill(&gui, bill_vertices, bill_faces, bill_center, transforms, eye);

	RenderDataInput bill_pass_input;
	bill_pass_input.assign(0, "vertex_position", bill_vertices.data(), bill_vertices.size(), 4, GL_FLOAT);
	bill_pass_input.assign(1, "center", bill_center.data(), bill_center.size(), 1, GL_FLOAT);
	bill_pass_input.assign_index(bill_faces.data(), bill_faces.size(), 3);
	RenderPass bill_pass(-1,
			bill_pass_input,
			{ bill_vertex_shader, nullptr, bill_fragment_shader},
			{ std_model, std_view, std_proj},
			{ "color" }
			);

	//End Billboard Code//


	//Start Texture Code//

	// std::vector<glm::vec2> tex_uv;
	// tex_uv.push_back(glm::vec2(1.0f, 1.0f));
	// tex_uv.push_back(glm::vec2(0.0f, 1.0f));
	// tex_uv.push_back(glm::vec2(0.0f, 0.0f));
	// tex_uv.push_back(glm::vec2(1.0f, 1.0f));
	// tex_uv.push_back(glm::vec2(0.0f, 0.0f));
	// tex_uv.push_back(glm::vec2(1.0f, 0.0f));

	// RenderDataInput tex_pass_input;
	// tex_pass_input.assign(0, "vertex_position", bill_vertices.data(), bill_vertices.size(), 4, GL_FLOAT);
	// tex_pass_input.assign(1, "uv", tex_uv.data(), tex_uv.size(), 2, GL_FLOAT);
	// tex_pass_input.assign_index(bill_faces.data(), bill_faces.size(), 3);
	// RenderPass tex_pass(-1,
	// 		tex_pass_input,
	// 		{ bill_vertex_shader, nullptr, tex_fragment_shader},
	// 		{ std_model, std_view, std_proj},
	// 		{ "color" }
	// 		);

	// Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
        // Positions          // Colors           // Texture Coords
         5.0f,  5.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
         5.0f, -5.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom Right
        -5.0f, -5.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
        -5.0f,  5.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // Top Left 
    };

    GLuint indices[] = {  // Note that we start from 0!
        1, 0, 2, // First Triangle
        2, 0, 3  // Second Triangle
    };

	GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    const std::vector<const char*> shaders = { bill_vertex_shader, nullptr, bill_fragment_shader};
	unsigned sampler2d_;
	unsigned vs_ = 0, gs_ = 0, fs_ = 0;
	unsigned sp_ = 0;

	// Program first  
	vs_ = compileShader(shaders[0], GL_VERTEX_SHADER);
	gs_ = compileShader(shaders[1], GL_GEOMETRY_SHADER);
	fs_ = compileShader(shaders[2], GL_FRAGMENT_SHADER);
	CHECK_GL_ERROR(sp_ = glCreateProgram());
	glAttachShader(sp_, vs_);
	glAttachShader(sp_, fs_);
	if (shaders[1])
		glAttachShader(sp_, gs_);

	glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 4 * bill_vertices.size(), bill_vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
	CHECK_GL_ERROR(glBindAttribLocation(sp_, 0, "position"));

	// glBindBuffer(GL_ARRAY_BUFFER, VBO);
 //    glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 16 * transforms.size(), transforms.data(), GL_STATIC_DRAW);

 //    // Transform attribute
 //    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
 //    glEnableVertexAttribArray(0);
	// CHECK_GL_ERROR(glBindAttribLocation(sp_, 0, "b_view"));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float)* 3 * bill_faces.size(), bill_faces.data(), GL_STATIC_DRAW);
    CHECK_GL_ERROR(glBindFragDataLocation(sp_, 0, "color"));

    glLinkProgram(sp_);
	CHECK_GL_PROGRAM_ERROR(sp_);


    // Get the uniform locations.
	GLint projection_matrix_location = 0;
	CHECK_GL_ERROR(projection_matrix_location =
			glGetUniformLocation(sp_, "projection"));
	GLint view_matrix_location = 0;
	CHECK_GL_ERROR(view_matrix_location =
			glGetUniformLocation(sp_, "view"));

    // glBindVertexArray(0); // Unbind VAO

	//Code above taken from render_pass

	//Following code developed using online tutorial
	Image img;
	unsigned char *data;
	std::string filename = "/v/filer4b/v38q001/rohitven/Desktop/CS354/A4/explosions/assets/textures/cloud_14.jpg";
	int pass = LoadJPEG(filename, &img);
	std::cout<<"\ndata size: "<<img.bytes.size();
	std::cout<<"\ndat1: "<<img.bytes[0];
	std::cout<<"\npass: "<<pass<<"\n";  //Finished grabbing JPG data!!
	data = img.bytes.data();
	std::cout<<"\ndata size: "<<sizeof(data);

    // Load and create a texture 
    GLuint texture1;
    std::cout<<"\nCreated a texture!";
    // ====================
    // Texture 1
    // ====================
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    std::cout<<"\nSet texture parameters!";
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    std::cout<<"\nSet texture filtering!";    
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bytes.data());

    std::cout<<"\nSet TexImage2D!!";
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.


	//End Texture Code//

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
			CHECK_GL_ERROR(glBindVertexArray(VAO));

			glUseProgram(sp_);

			// // Pass uniforms in.
			// CHECK_GL_ERROR(glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE,
			// 		&projection_matrix[0][0]));
			// CHECK_GL_ERROR(glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE,
			// 		&view_matrix[0][0]));
			std::vector<ShaderUniform> uniforms_ = {std_proj, std_view};
			const std::vector<unsigned> unilocs_ = {projection_matrix_location, view_matrix_location};
			bind_uniforms(uniforms_, unilocs_);
			bill_vertices.clear();
			transforms.clear();
			eye = gui.getCamera();
			bill_center.clear();
			create_bill(&gui, bill_vertices, bill_faces, bill_center, transforms, eye);
			// Bind Textures using texture units
	        // glActiveTexture(GL_TEXTURE0);
	        // glBindTexture(GL_TEXTURE_2D, texture1);
	        // glUniform1i(glGetUniformLocation(sp_, "ourTexture1"), 0);
			// bill_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, bill_faces.size() * 3, GL_UNSIGNED_INT, 0));
			// glBindVertexArray(0);

			floor_pass.setup();
			// Draw our triangles.
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, floor_faces.size() * 3, GL_UNSIGNED_INT, 0));

			// tex_pass.setup();
			// CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, bill_faces.size() * 3, GL_UNSIGNED_INT, 0));			
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
