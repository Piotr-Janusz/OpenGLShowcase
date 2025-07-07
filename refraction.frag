#version 330 core
out vec4 FragColor;

in vec3 nor;
in vec3 FragPosWorldSpace;
in vec2 tex;

uniform vec3 camPos;
uniform samplerCube skybox;

void main()
{   
    float ratio = 1.00 / 1.6;
    vec3 I = normalize(FragPosWorldSpace - camPos);
    vec3 R = refract(I, normalize(nor), ratio);
    FragColor = vec4(texture(skybox, R).rgb, 1);
}