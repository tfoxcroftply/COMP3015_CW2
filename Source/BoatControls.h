#pragma once

#include "scenebasic_uniform.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <ctime>

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

int CurrentTime() {
	unsigned int Tick = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	return Tick;
}

struct CameraData {
	glm::vec3 CameraPosition;
	glm::mat4 ViewMatrix;
};

class Boat {
private:
	float BoatHeight = 0.5f;
	float BoatSpeed = 2.0f;
	float BoatRotateSensitivity = 1.0f;
	float BoatScale = 0.02f;
	float Smoothing = 0.004f;
	float RotationSmoothing = 0.003f;

	glm::vec3 LastCameraPosition = glm::vec3(0.0f);
	glm::vec3 LastCameraOrientation = glm::vec3(0.0f);

	bool IsAccelerating = false;
	int LastBoatAccelStart = 0;
	int AccelSpeed = 1;
	float LastBoatSpeed = 0.0f;

	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);

	glm::mat4 BoatMatrix = glm::mat4(1.0f);
public:
	glm::vec3 Position = glm::vec3(-5.0f, 0.5f, 0.0f);
	glm::mat4 Projection;
	GLFWwindow* Window;

	Boat() {};

	void Update(float Delta) {
		if (Window != NULL) {
			float CalcSpeed = 1.0f * Delta;
			float InverseScale = 1.0f / BoatScale;
			if (KeyPress(GLFW_KEY_W)) {
				if (!IsAccelerating) {
					IsAccelerating = true;
					LastBoatAccelStart = CurrentTime();
				} else {
					int TimeSinceStart = CurrentTime() - LastBoatAccelStart; // eg 2500
					if (TimeSinceStart != 0) {
						float AccelMultiplier = float(TimeSinceStart) / (AccelSpeed * 1000.0f);
						AccelMultiplier = Clamp(AccelMultiplier, 0.0f, 1.0f);
						LastBoatSpeed = AccelMultiplier;
						BoatMatrix = glm::translate(BoatMatrix, glm::vec3(0.0f, BoatSpeed * CalcSpeed * InverseScale * AccelMultiplier, 0.0f));
					}
				}
			}
			else if (!KeyPress(GLFW_KEY_W)) {
				IsAccelerating = false;
			}
			if (KeyPress(GLFW_KEY_A)) {
				BoatMatrix = glm::rotate(BoatMatrix, glm::radians(BoatRotateSensitivity * CalcSpeed * InverseScale * LastBoatSpeed), glm::vec3(0.0f, 0.0f, 1.0f));
			}
			if (KeyPress(GLFW_KEY_D)) {
				BoatMatrix = glm::rotate(BoatMatrix, glm::radians(BoatRotateSensitivity * CalcSpeed * InverseScale * LastBoatSpeed), glm::vec3(0.0f, 0.0f, -1.0f));
			}
		}
	}

	void Init() {
		BoatMatrix = glm::scale(BoatMatrix, glm::vec3(BoatScale));
		BoatMatrix = glm::rotate(BoatMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::mat4 GetBoatMatrix() {
		return BoatMatrix;
	}

	glm::vec3 GetPositionFromMatrix(glm::mat4 Input) {
		return glm::vec3(Input[3][0], Input[3][1], Input[3][2]);
	}
	
	glm::vec3 GetBoatPosition() {
		return GetPositionFromMatrix(BoatMatrix);
	}

	CameraData GetCameraData() {
		float InverseScale = 1.0f / BoatScale; // get inverse of boat scale for reverting

		glm::mat4 NewMatrix = BoatMatrix;
		NewMatrix = glm::rotate(NewMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // returns back to original state
		NewMatrix = glm::scale(NewMatrix, glm::vec3(InverseScale)); // return to normal size

		glm::vec3 BoatPosition = glm::vec3(NewMatrix[3]); // extract position from boat matrix
		glm::mat3 RotationMatrix = glm::mat3(NewMatrix); // extract rotation from boat matrix

		glm::vec3 CameraPosition = BoatPosition - (RotationMatrix * (glm::vec3(0.0f, -0.4f, -1.1f))); // get the camera position of the boat ensuring to move relative to the boat
		glm::vec3 CameraUp = RotationMatrix * glm::vec3(0.0f, 1.0f, 0.0f); // copy the boats up vector
		glm::vec3 CameraTarget = glm::vec3(BoatPosition.x, BoatPosition.y + 0.4f, BoatPosition.z); // where the camera aims at

		if (LastCameraPosition != glm::vec3(0.0f) && LastCameraOrientation != glm::vec3(0.0f)) {
			CameraPosition = glm::mix(LastCameraPosition, CameraPosition, Smoothing);
			CameraTarget = glm::mix(LastCameraOrientation, CameraTarget, RotationSmoothing);
		}

		LastCameraPosition = CameraPosition;
		LastCameraOrientation = CameraTarget;
		glm::mat4 View = glm::lookAt(CameraPosition, CameraTarget, CameraUp); // final calculation

		return {CameraPosition, View};
	}

	bool KeyPress(int Input) {
		if (glfwGetKey(Window, Input) == GLFW_PRESS) { // Ensuring that it has actually been pressed
			return true;
		}
		return false;
	}
};