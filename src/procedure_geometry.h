#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>
#include "gui.h"

class LineMesh;

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces);
void create_bill(GUI *g, std::vector<glm::vec4>& bill_vertices, std::vector<glm::uvec3>& bill_faces, std::vector<glm::vec4> bill_center, std::vector<glm::mat4>& transforms, glm::vec3 center);
// FIXME: Add functions to generate the bone mesh.

#endif
