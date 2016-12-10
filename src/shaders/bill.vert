R"zzz(
#version 330 core
uniform mat4 projection;
uniform mat4 view;
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;
in vec4 center;
in mat4 b_view;
out vec2 TexCoord;
void main()
{
	mat4 temp = view;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			if(i == j)
			{
				temp[i][j] = 1;
			}
			else
			{
				temp[i][j] = 0;
			}
		}
	}
	vec4 c = projection * view * center;
	//vec4 cam_up = vec4(view[0][1], view[1][1], view[2][1], view[3][1]);
	//vec4 cam_r = vec4(view[0][0], view[1][0], view[2][0], view[3][0]);
	//vec4 p = c + cam_r * position.x * 100 + cam_up * position.y * 100;
	gl_Position = projection * view * (c);
	//gl_Position = temp * gl_Position;
	gl_Position /= gl_Position.w;
	gl_Position.xy += position.xy * vec2(0.2, 0.05);
}
)zzz"