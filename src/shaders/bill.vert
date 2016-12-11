R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 view;
in vec4 position;
in vec2 uv;
in vec4 center;
in mat4 b_view;
out vec2 TexCoord;
void main()
{
	gl_Position = projection * view * position;
	TexCoord = vec2(uv.x, 1.0 - uv.y);
}
)zzz"