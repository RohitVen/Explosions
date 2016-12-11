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

void create_bill(GUI *g, std::vector<glm::vec4>& bill_vertices, std::vector<glm::uvec3>& bill_faces, std::vector<glm::vec4> &bill_center, float scale, float rot)
{
	int index = 0;
	for(int i = 0; i < bill_center.size(); i++)
	{
		glm::vec4 t = bill_center[i];
		// t = g->projection_matrix_ * g->view_matrix_ * t;
		// glm::vec4 t1 = glm::vec4(-0.5,0.5,1.0,1.0);
		// glm::vec4 t2 = glm::vec4(0.5,0.5,1.0,1.0);
		// glm::vec4 t3 = glm::vec4(0.5,-0.5,1.0,1.0);
		// glm::vec4 t4 = glm::vec4(-0.5,-0.5,1.0,1.0);
		glm::vec4 tang = glm::vec4(g->view_matrix_[0][0],g->view_matrix_[1][0],g->view_matrix_[2][0],0);
		glm::vec4 up = glm::vec4(g->view_matrix_[0][1],g->view_matrix_[1][1],g->view_matrix_[2][1],0);
		glm::vec3 center = g->getCamera();

		glm::mat4 r = glm::rotate(rot, g->look_);

		tang *= scale;
		up *= scale;

		glm::vec4 t1 = (glm::vec4(0) - tang) + up;
		glm::vec4 t2 = (glm::vec4(0) + tang) + up;
		glm::vec4 t3 = (glm::vec4(0) + tang) - up;
		glm::vec4 t4 = (glm::vec4(0) - tang) - up;

		t1 = (r * t1)+t;
		t2 = (r * t2)+t;
		t3 = (r * t3)+t;
		t4 = (r * t4)+t;

		/*Debugging stuff beloooooooww
		glm::vec4 f1 = g->projection_matrix_ * g->view_matrix_ * t1;
		glm::vec4 f2 = g->projection_matrix_ * g->view_matrix_ * t2;
		glm::vec4 f3 = g->projection_matrix_ * g->view_matrix_ * t3;
		glm::vec4 f4 = g->projection_matrix_ * g->view_matrix_ * t4;

		std::cout<<"\nworld center: "<<g->center_.x<<" "<<g->center_.y<<" "<<g->center_.z;
		std::cout<<"\neye: "<<center.x<<" "<<center.y<<" "<<center.z;
		std::cout<<"\ntang: "<<tang.x<<" "<<tang.y<<" "<<tang.z;
		std::cout<<"\nup: "<<up.x<<" "<<up.y<<" "<<up.z;
		std::cout<<"\nlook: "<<g->look_.x<<" "<<g->look_.y<<" "<<g->look_.z;
		std::cout<<"\nbill center: "<<t.x<<" "<<t.y<<" "<<t.z;
		std::cout<<"\n";

		std::cout<<"\nt1: "<<t1.x<<" "<<t1.y<<" "<<t1.z<<" "<<t1.w;
		std::cout<<"\nt2: "<<t2.x<<" "<<t2.y<<" "<<t2.z<<" "<<t2.w;
		std::cout<<"\nt3: "<<t3.x<<" "<<t3.y<<" "<<t3.z<<" "<<t3.w;
		std::cout<<"\nt4: "<<t4.x<<" "<<t4.y<<" "<<t4.z<<" "<<t4.w;
		std::cout<<"\n";

		std::cout<<"\nf1: "<<f1.x<<" "<<f1.y<<" "<<f1.z<<" "<<f1.w;
		std::cout<<"\nf2: "<<f2.x<<" "<<f2.y<<" "<<f2.z<<" "<<f2.w;
		std::cout<<"\nf3: "<<f3.x<<" "<<f3.y<<" "<<f3.z<<" "<<f3.w;
		std::cout<<"\nf4: "<<f4.x<<" "<<f4.y<<" "<<f4.z<<" "<<f4.w;

		std::cout<<"\n\n";*/

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
