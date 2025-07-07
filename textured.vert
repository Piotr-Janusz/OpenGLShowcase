#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aTex;
layout(location = 2) in vec3 aNor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int water;
uniform float time;

out vec2 tex;
out vec3 nor;
out vec3 FragPosWorldSpace;

mat4 translate(vec3 d)
{
	return mat4(1, 0, 0, d.x,
	            0, 1, 0, d.y,
	            0, 0, 1, d.z,
	            0, 0, 0, 1);
}


void main()
{
	//mat4 translation = translate(vec3(sin(aPos.y + time), sin(aPos.x + time), 0));
	mat4 translation = translate(vec3(sin((aPos.y / 2) + time), cos((aPos.x / 2) + time), 0));
	
	tex = aTex.xy;

	nor = mat3(transpose(inverse(model))) * aNor;
	FragPosWorldSpace = vec3(model * vec4(aPos,1.0f));

	if(water == 0)
	{
		gl_Position = projection * view * (model) * (vec4(aPos, 1.f));
	}
	else if(water == 1)
	{
		gl_Position = projection * view * (model) * (vec4(aPos, 1.f) * translation);
	}
	
}
