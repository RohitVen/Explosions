R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 view;
in vec4 position;
in vec2 uv;
out vec2 uv_coords;
void main()
{
	vec4 center = vec4(0,0,0,1);
	center = projection * view * center;
	vec4 cam_up = vec4(view[0][1], view[1][1], view[2][1], view[3][1]);
	vec4 cam_r = vec4(view[0][0], view[1][0], view[2][0], view[3][0]);
	vec4 p = center + cam_r * position.x * 100 + cam_up * position.y * 100;
	gl_Position = projection * view * p;
	uv_coords = uv;
}
)zzz"