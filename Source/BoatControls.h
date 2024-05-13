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
	if (value < min) { return min; }
	if (value > max) { return max; }
	return value;
}

float Lerp(float Start, float End, float Strength) { // not sure what to call it for normal numbers so it is just named as lerp
	return Start + (End - Start) * Strength;
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
	float BoatSpeed = 1.6f;
	float SlowDownMultiplier = 0.5f;
	float BoatRotateSensitivity = 1.0f;
	float BoatScale = 0.02f;
	float Smoothing = 0.04f;
	float RotationSmoothing = 0.04f;

	glm::vec3 LastCameraPosition = glm::vec3(0.0f);
	glm::vec3 LastCameraOrientation = glm::vec3(0.0f);

	bool IsAccelerating = false;
	int LastBoatAccelStart = 0;

	float AccelSpeed = 0.5f;
	float BoatThrottle = 0.0f;

	float SteerAccelSpeed = 1.5f;
	float BoatSteerSpeed = 2.0f;
	float BoatSteerLeft = 0.0f;
	float BoatSteerRight = 0.0f;

	float SteerWobbleDegrees = 15.0f;
	float SteerWobbleAccel = 1.5f;
	float SteerWobbleLeft = 0.0f;
	float SteerWobbleRight = 0.0f;
	float LastWobble = 0.0f;

	int LastMouseX = -1;
	int LastMouseY = -1;
	float Yaw = 0;
	float Pitch = 0;
	float LastYaw = 0;
	float LastPitch = 0;
	float AimRetractionStrength = 2.0f;
	float LastMoveTick = 0.0f;
	float RetractionDelay = 1.0f;
	float MouseSensitivity = 0.03f;

	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);


	glm::mat4 BoatMatrixPreWobble = glm::mat4(1.0f);
public:
	glm::vec3 Position = glm::vec3(-5.0f, 0.5f, 0.0f);
	glm::mat4 Projection;
	glm::mat4 BoatMatrix = glm::mat4(1.0f);
	GLFWwindow* Window;

	Boat() {};

	void Update(float Delta) {
		if (Window != NULL) {
			if (BoatMatrixPreWobble != glm::mat4(1.0f)) {
				BoatMatrix = BoatMatrixPreWobble;
			}

			float CalcSpeed = 1.0f * Delta;
			float InverseScale = 1.0f / BoatScale;

			if (KeyPress(GLFW_KEY_W)) {
				BoatThrottle = Clamp(BoatThrottle + (AccelSpeed * CalcSpeed), 0.0f, 1.0f);
			} else {
				BoatThrottle = Clamp(BoatThrottle - (AccelSpeed * CalcSpeed * SlowDownMultiplier), 0.0f, 1.0f);
			}
			if (KeyPress(GLFW_KEY_A)) {
				BoatSteerLeft = Clamp(BoatSteerLeft + (SteerAccelSpeed * CalcSpeed), -1.0f, 0.0f);
				SteerWobbleLeft = Clamp(SteerWobbleLeft + (SteerWobbleAccel * CalcSpeed), -1.0f, 0.0f);
			} else {
				BoatSteerLeft = Clamp(BoatSteerLeft - (SteerAccelSpeed * CalcSpeed), -1.0f, 0.0f);
				SteerWobbleLeft = Clamp(SteerWobbleLeft - (SteerWobbleAccel * CalcSpeed), -1.0f, 0.0f);
			}
			if (KeyPress(GLFW_KEY_D)) {
				BoatSteerRight = Clamp(BoatSteerRight - (SteerAccelSpeed * CalcSpeed), 0.0f, 1.0f);
				SteerWobbleRight = Clamp(SteerWobbleRight - (SteerWobbleAccel * CalcSpeed), 0.0f, 1.0f);
			} else {
				BoatSteerRight = Clamp(BoatSteerRight + (SteerAccelSpeed * CalcSpeed), 0.0f, 1.0f);
				SteerWobbleRight = Clamp(SteerWobbleRight + (SteerWobbleAccel * CalcSpeed), 0.0f, 1.0f);
			}

			float FinalSteer = (BoatSteerLeft + BoatSteerRight) / 2;

			BoatMatrix = glm::rotate(BoatMatrix, glm::radians(FinalSteer * CalcSpeed * InverseScale * BoatThrottle * BoatSteerSpeed), glm::vec3(0.0f, 0.0f, 1.0f));
			BoatMatrix = glm::translate(BoatMatrix, glm::vec3(0.0f, BoatSpeed * CalcSpeed * InverseScale * BoatSpeed * BoatThrottle, 0.0f));
			BoatMatrixPreWobble = BoatMatrix;

			float FinalSteerWobble = (SteerWobbleLeft + SteerWobbleRight) / 2;
			float CalcRotation = -FinalSteerWobble * SteerWobbleDegrees * BoatThrottle;

			LastWobble = glm::mix(LastWobble, CalcRotation, 0.01f * 144.0f * Delta); // this prevents sudden stopping of rotation

			BoatMatrix = glm::rotate(BoatMatrix, glm::radians(LastWobble), glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	void UpdateMouse(double X, double Y) {
		if (LastMouseX == -1 or LastMouseY == -1) {
			LastMouseX = X;
			LastMouseY = Y;
			return;
		}

		float XChange = X - LastMouseX;
		float YChange = Y - LastMouseY;

		XChange *= MouseSensitivity;
		YChange *= MouseSensitivity;

		Yaw = Clamp(Yaw + float(XChange),-60.0f,60.0f);
		Pitch = Clamp(Pitch + float(YChange), -10.0f, 20.0f);

		LastMouseX = X;
		LastMouseY = Y;

		LastMoveTick = CurrentTime();

		std::cout << Yaw;
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

	CameraData GetCameraData(float Delta) {
		float InverseScale = 1.0f / BoatScale; // get inverse of boat scale for reverting

		if (CurrentTime() > (RetractionDelay * 1000) + LastMoveTick) {
			Yaw = Lerp(Yaw, 0.0f, AimRetractionStrength * Delta);
			//Pitch = Lerp(Pitch, 0.0f, AimRetractionStrength * Delta);
		}

		glm::mat4 NewMatrix = BoatMatrix;
		NewMatrix = glm::rotate(NewMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // returns back to original state

		//NewMatrix = glm::rotate(NewMatrix, glm::radians(-Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
		NewMatrix = glm::rotate(NewMatrix, glm::radians(-Yaw), glm::vec3(0.0f, 1.0f, 0.0f));

		NewMatrix = glm::scale(NewMatrix, glm::vec3(InverseScale)); // return to normal size

		glm::vec3 BoatPosition = glm::vec3(NewMatrix[3]); // extract position from boat matrix
		glm::mat3 RotationMatrix = glm::mat3(NewMatrix); // extract rotation from boat matrix

		glm::vec3 CameraPosition = BoatPosition - (RotationMatrix * (glm::vec3(0.0f, -0.4f, -1.1f))); // get the camera position of the boat ensuring to move relative to the boat
		glm::vec3 CameraUp = RotationMatrix * glm::vec3(0.0f, 1.0f, 0.0f); // copy the boats up vector
		glm::vec3 CameraTarget = glm::vec3(BoatPosition.x, BoatPosition.y + 0.4f, BoatPosition.z); // where the camera aims at

		if (LastCameraPosition != glm::vec3(0.0f) && LastCameraOrientation != glm::vec3(0.0f)) {
			CameraPosition = glm::mix(LastCameraPosition, CameraPosition, (Smoothing * 144.0f) * Delta); // values were picked in 144hz testing so use it as a base
			CameraTarget = glm::mix(LastCameraOrientation, CameraTarget, (RotationSmoothing * 144.0f) * Delta); 
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