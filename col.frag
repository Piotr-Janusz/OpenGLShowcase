#version 450 core

layout (location = 0) out vec4 fColour;

in vec3 col;


void main()
{
	fColour = vec4(0f, 1.0f, 0f, 1.f);
}
