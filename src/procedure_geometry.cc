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

void create_bill(GUI *g, std::vector<glm::vec4>& bill_vertices, std::vector<glm::uvec3>& bill_faces, std::vector<glm::vec4> bill_center, std::vector<glm::mat4>& transforms, glm::vec3 center)
{
	int index = 0;
	for(int i = 0; i < bill_center.size(); i++)
	{
		glm::vec4 t = bill_center[i];
		t = g->projection_matrix_*g->view_matrix_*t;
		glm::vec4 t1 = glm::vec4(t.x-0.5,t.y+0.25,1.0,1.0);
		glm::vec4 t2 = glm::vec4(t.x+0.5,t.y+0.25,1.0,1.0);
		glm::vec4 t3 = glm::vec4(t.x+0.5,t.y-0.25,1.0,1.0);
		glm::vec4 t4 = glm::vec4(t.x-0.5,t.y-0.25,1.0,1.0);

		bill_vertices.push_back(t1);
		bill_vertices.push_back(t2);
		bill_vertices.push_back(t3);
		bill_vertices.push_back(t4);

		bill_faces.push_back(glm::uvec3(index+1,index,index+2));
		bill_faces.push_back(glm::uvec3(index+2,index,index+3));
		index += 4;
	}
	// bill_vertices.push_back(glm::vec4(-5.0, 5.0, 1.0, 1.0));
	// bill_vertices.push_back(glm::vec4(5.0, 5.0, 1.0, 1.0));
	// bill_vertices.push_back(glm::vec4(5.0, 0.0, 1.0, 1.0));
	// bill_vertices.push_back(glm::vec4(-5.0, 0.0, 1.0f, 1.0));
	// bill_faces.push_back(glm::uvec3(1,0,2));
	// bill_faces.push_back(glm::uvec3(2,0,3));
}

// FIXME: create cylinders and lines for the bones
// Hints: Generate a lattice in [-0.5, 0, 0] x [0.5, 1, 0] We wrap this
// around in the vertex shader to produce a very smooth cylinder.  We only
// need to send a small number of points.  Controlling the grid size gives a
// nice wireframe.
