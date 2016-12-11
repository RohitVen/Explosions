R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 view;
in vec4 position;
in vec2 uv;
out vec2 TexCoord;
void main()
{
	gl_Position = projection * view * position;
	TexCoord = vec2(uv.x, 1-uv.y);
}
)zzz"