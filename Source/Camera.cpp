#include "Camera.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <algorithm>
#include <iostream>

void Camera::Update(float Delta) {
	if (Window != NULL) {
		float calcSpeed = 1.0f * Delta;
		if (KeyPress(GLFW_KEY_W)) {
			Position += calcSpeed * Front;
		}
		if (KeyPress(GLFW_KEY_A)) {
			Position -= normalize(cross(Front, Up)) * calcSpeed;
		}
		if (KeyPress(GLFW_KEY_S)) {
			Position -= calcSpeed * Front;
		}
		if (KeyPress(GLFW_KEY_D)) {
			Position += normalize(cross(Front, Up)) * calcSpeed;
		}
		if (KeyPress(GLFW_KEY_Q)) {
			Position -= calcSpeed * Up;
		}
		if (KeyPress(GLFW_KEY_E)) {
			Position += calcSpeed * Up;
		}
	}
}

float Clamp(float value, float min, float max) {
	if (value < min) {
		return min;
	}
	else if (value > max) {
		return max;
	}
	else {
		return value;
	}
}

void Camera::MouseInput(double XPos, double YPos) { // parse mouse movements
	if (!MouseHasMoved)
	{
		LastMouseX = XPos;
		LastMouseY = YPos;
		MouseHasMoved = true; // for setting the first movement as baseline
	}

	float xChange = XPos - LastMouseX; // apply changes
	float yChange = LastMouseY - YPos;
	LastMouseX = XPos;
	LastMouseY = YPos;

	xChange *= Sensitivity; // sensitivity as multiplier
	yChange *= Sensitivity;

	MouseYaw += xChange; // apply changes
	MousePitch += yChange;

	MousePitch = Clamp(MousePitch, -89.9f, 89.9f); // must still use std:: for some reason. limits camera movement

	Front = glm::normalize(glm::vec3( // sets the new aiming vector
		cos(glm::radians(MouseYaw)) * cos(glm::radians(MousePitch)),
		sin(glm::radians(MousePitch)),
		sin(glm::radians(MouseYaw)) * cos(glm::radians(MousePitch))
	));
}


glm::mat4 Camera::GetViewMatrix() { // encapsulated view matrix
	glm::mat4 view = glm::lookAt(Position, Position + Front, Up);
	return view;
}
