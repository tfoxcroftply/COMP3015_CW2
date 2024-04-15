#pragma once

#include "scenebasic_uniform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

struct Camera {
private:
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

	float LastMouseX, LastMouseY;
	float MouseYaw, MousePitch;
	bool MouseHasMoved = false;
	float Speed = 1.0f;
	float Sensitivity = 0.1f;
public:
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 Projection;
	GLFWwindow* Window;

	Camera() {};
	void Update(float Delta);
	glm::mat4 GetViewMatrix();
	void MouseInput(double XPos, double YPos);
	bool KeyPress(int Input) {
		if (glfwGetKey(Window, Input) == GLFW_PRESS) { // Ensuring that it has actually been pressed
			return true;
		}
		return false;
	}
};