#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>

class LineMesh;

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces);
void create_bill(std::vector<glm::vec4>& bill_vertices, std::vector<glm::uvec3>& bill_faces);
// FIXME: Add functions to generate the bone mesh.

#endif
