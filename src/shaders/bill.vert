R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 view;
in vec4 position;
in vec4 color;
in float alpha;
out vec4 fragColor;
void main()
{
	gl_Position = projection * view * position;
	//TexCoord = vec2(uv.x, 1-uv.y);
	fragColor = vec4(color.x, color.y, color.z, alpha);
}
)zzz"