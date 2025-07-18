#version 450 core

layout(location = 0) in vec4 vPos;
layout(location = 1) in vec3 vCol;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 col;

void main()
{
	gl_Position = projection * view * model * vPos;
	col = vCol;
}
