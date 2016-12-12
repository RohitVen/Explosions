R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 view;
in vec4 position;
in vec4 color;
in vec2 uv;
in float rgb[];
out vec2 TexCoord;
out vec4 fragColor;
void main()
{
	gl_Position = projection * view * position;
	TexCoord = vec2(uv.x, 1-uv.y);
	fragColor = vec4(rgb[0], rgb[1], rgb[2], 1);
}
)zzz"