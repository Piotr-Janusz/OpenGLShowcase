#version 330 core

uniform sampler2D Texture;
uniform samplerCube depthMap;
uniform samplerCube skybox;

in vec3 nor;
in vec3 FragPosWorldSpace;
in vec2 tex;

uniform vec3 camPos;
uniform vec3 lightPos;

uniform float far_plane;

uniform int water;

out vec4 fragColour;

#define NR_POINT_LIGHTS 1


float FindShadow(vec3 FragPos)
{
	vec3 to_light = FragPos - lightPos;
	float closestDepth = texture(depthMap, to_light).r;
	closestDepth *= far_plane;
	float currentDepth = length(to_light);
	float bias = 0.05;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;



	return shadow;
}


vec3 calculatePositionalIllumination(vec3 viewDir)
{
	// Get the direction between the fragment position and the light position
	vec3 Nto_light = normalize(lightPos - FragPosWorldSpace);
	vec3 Nnor = normalize(nor);
	// calculate the ambient
	vec3 lightColor = vec3(0.4);
	vec3 ambient = 0.05 * lightColor;

	vec3 colour = texture(Texture, tex).rgb;
	
	// calculate the diffuse
	float diff = max(dot(Nto_light, Nnor), 0.0f);
	vec3 diffuse = diff * lightColor;
	
	// Specular
	vec3 viewDirection = normalize(camPos - FragPosWorldSpace);
	vec3 reflectedDirection = reflect(-Nto_light, Nnor);
	float spec = 0.0;
	vec3 halfway = normalize(Nto_light + viewDirection);
	spec = pow(max(dot(Nnor, halfway), 0),64);
	vec3 specular = spec * lightColor;

	float shadow = FindShadow(FragPosWorldSpace);
	//float shadow = 0;


	//return (ambient + diffuse + specular);
	return (ambient + (1.0 - shadow) * (diffuse + specular)) * colour; 
}

vec3 CalcPointLight(vec3 lightPosition, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(lightPosition - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    // attenuation
    float distance = length(lightPosition - fragPos);
	// 1.0 / (light.constant + light.linear * distance + light.quadratic)
    float attenuation = 1.0 / (1.0 + 0.14 * distance + 0.01 * (distance * distance));    
    // combine results
    vec3 ambient = 0.1 * vec3(texture(Texture, tex));
    vec3 diffuse = vec3(1f, 0.1f,0.1f) * diff * vec3(texture(Texture, tex));
    vec3 specular = vec3(0.2f,0.2f,0.2f) * spec * vec3(texture(Texture, tex));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}


void main()
{
	vec3 lightDirection = normalize(lightPos - FragPosWorldSpace);
	vec3 viewDir = normalize(camPos - FragPosWorldSpace);
	vec4 result = vec4(0f,0f,0f, 0f);
	float textureAlpha = texture(Texture, tex).w;
	vec3 temp = CalcPointLight(vec3(17.8793,-0.92803,-1.88456), normalize(nor), FragPosWorldSpace, viewDir);
	vec3 temp2 = CalcPointLight(vec3(8.55814,-1.85559,28.6543), normalize(nor), FragPosWorldSpace, viewDir);
	if(water == 1)
	{
		float ratio = 1.00 / 1.33;
		vec3 I = normalize(FragPosWorldSpace - camPos);
		vec3 R = refract(I, normalize(nor), ratio);
		vec3 mixed = mix(calculatePositionalIllumination(viewDir), texture(skybox, R).rgb, 0.1);
		result = vec4(mixed, 0.7f);
		result = result + vec4(temp, 1f);
		result = result + vec4(temp2, 1f);
		result.a = 0.8f;
	}
	else if(water == 0)
	{
		result = vec4(calculatePositionalIllumination(viewDir), 1f);
		result = result + vec4(temp, 1f);
		result = result + vec4(temp2, 1f);
	}
	
	//result += CalcPointLight(vec3(17.8793,-0.92803,-1.88456), normalize(nor), FragPosWorldSpace, viewDir);
	
	fragColour = result;
	//fragColour = vec4(vec3(FindShadow(FragPosWorldSpace)), 1.0f);
}







