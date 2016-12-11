R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 view;
layout (location = 0) in vec4 position;
in vec4 center;
in mat4 b_view;
out vec2 TexCoord;
void main()
{
	gl_Position = projection * view * position;
}
)zzz"