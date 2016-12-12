#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"
#include "gui.h"

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces)
{
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	floor_faces.push_back(glm::uvec3(0, 1, 2));
	floor_faces.push_back(glm::uvec3(2, 3, 0));
}

void create_bill(GUI *g, std::vector<glm::vec4>& bill_vertices, std::vector<glm::uvec3>& bill_faces, std::vector<glm::vec4> &bill_center, std::vector<float> scale, std::vector<float> rot)
{
	int index = 0;
	for(int i = 0; i < bill_center.size(); i++)
	{
		glm::vec4 t = bill_center[i];
		glm::vec4 tang = glm::vec4(g->view_matrix_[0][0],g->view_matrix_[1][0],g->view_matrix_[2][0],0);
		glm::vec4 up = glm::vec4(g->view_matrix_[0][1],g->view_matrix_[1][1],g->view_matrix_[2][1],0);
		glm::vec3 center = g->getCamera();

		glm::mat4 r = glm::rotate(rot[i], g->look_);

		tang *= scale[i];
		up *= scale[i];

		glm::vec4 t1 = (glm::vec4(0) - tang) + up;
		glm::vec4 t2 = (glm::vec4(0) + tang) + up;
		glm::vec4 t3 = (glm::vec4(0) + tang) - up;
		glm::vec4 t4 = (glm::vec4(0) - tang) - up;

		t1 = (r * t1)+t;
		t2 = (r * t2)+t;
		t3 = (r * t3)+t;
		t4 = (r * t4)+t;

		bill_vertices.push_back(t1);  //Top left
		bill_vertices.push_back(t2);  //Top right
		bill_vertices.push_back(t3);  //Bottom right
		bill_vertices.push_back(t4);  //Bottom left

		bill_faces.push_back(glm::uvec3(index+1,index,index+2));
		bill_faces.push_back(glm::uvec3(index+2,index,index+3));
		index += 4;
	}
}

// FIXME: create cylinders and lines for the bones
// Hints: Generate a lattice in [-0.5, 0, 0] x [0.5, 1, 0] We wrap this
// around in the vertex shader to produce a very smooth cylinder.  We only
// need to send a small number of points.  Controlling the grid size gives a
// nice wireframe.
