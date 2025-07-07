#pragma once

#include <stdio.h>
#include <glm/glm.hpp>

struct SCamera
{
	enum Camera_Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;

	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;

	const float MovementSpeed = 0.1f;
	float MouseSensitivity = 1.f;



};


void InitCamera(SCamera& in)
{
	in.Front = glm::vec3(0.0f, 0.0f, -1.0f);
	in.Position = glm::vec3(0.0f, 0.0f, 0.0f);
	in.Up = glm::vec3(0.0f, 1.0f, 0.0f);
	in.WorldUp = in.Up;
	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));

	in.Yaw = -90.f;
	in.Pitch = 0.f;
}

float cam_dist = 2.f;

void MoveAndOrientCamera(SCamera& in)
{
	in.Front.x = cos(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch));
	in.Front.y = sin(glm::radians(in.Pitch));
	in.Front.z = sin(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch));
	in.Front = glm::normalize(in.Front);
	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));
	in.Up = glm::normalize(glm::cross(in.Right, in.Front));
}

void ProcessMouseInputs(SCamera& in, float xOffset, float yOffset)
{
	in.Yaw += xOffset;
	in.Pitch += yOffset;

	if (in.Pitch > 89.0f)
	{
		in.Pitch = 89.0f;
	}
	if (in.Pitch < -89.0f)
	{
		in.Pitch = -89.0f;
	}
}