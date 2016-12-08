#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces)
{
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	floor_faces.push_back(glm::uvec3(0, 1, 2));
	floor_faces.push_back(glm::uvec3(2, 3, 0));
}

void create_bill(std::vector<glm::vec4>& bill_vertices, std::vector<glm::uvec3>& bill_faces)
{
	bill_vertices.push_back(glm::vec4(-5.0, 5.0, 1.0, 1.0));
	bill_vertices.push_back(glm::vec4(5.0, 5.0, 1.0, 1.0));
	bill_vertices.push_back(glm::vec4(5.0, 0.0, 1.0, 1.0));
	bill_vertices.push_back(glm::vec4(-5.0, 0.0, 1.0f, 1.0));
	bill_faces.push_back(glm::uvec3(1,0,2));
	bill_faces.push_back(glm::uvec3(2,0,3));
}

// FIXME: create cylinders and lines for the bones
// Hints: Generate a lattice in [-0.5, 0, 0] x [0.5, 1, 0] We wrap this
// around in the vertex shader to produce a very smooth cylinder.  We only
// need to send a small number of points.  Controlling the grid size gives a
// nice wireframe.
