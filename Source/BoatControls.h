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
#include <irrKlang.h>

float Clamp(float value, float min, float max) { // clamps a value between a range
	if (value < min) { return min; }
	if (value > max) { return max; }
	return value;
}

float Lerp(float Start, float End, float Strength) { // not sure what to call it for normal numbers so it is just named as lerp
	return Start + (End - Start) * Strength;
}

int CurrentTime() { // returns time in milliseconds
	unsigned int Tick = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	return Tick;
}

struct CameraData {
	glm::vec3 CameraPosition;
	glm::mat4 ViewMatrix;
};

class Boat {
private:
	int LastPendingMove = 0; // Boat parameters and trackers
	bool PendingMove = false;
	float BoatHeight = 0.5f;
	float BoatSpeed = 1.7f;
	float SlowDownMultiplier = 0.5f;
	float BoatRotateSensitivity = 1.0f;
	float BoatScale = 0.02f;
	float Smoothing = 0.04f;
	float RotationSmoothing = 0.04f;

	glm::vec3 LastCameraPosition = glm::vec3(0.0f);
	glm::vec3 LastCameraOrientation = glm::vec3(0.0f);
	glm::vec3 StartOrientation = glm::vec3(0.0f);

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
	int BoostDuration = 3;

	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);

	glm::mat4 BoatMatrixPreWobble = glm::mat4(1.0f);
public:
	float LastBoostTime = 0;
	glm::vec3 Position = glm::vec3(-5.0f, 0.5f, 0.0f);
	glm::mat4 Projection;
	glm::mat4 BoatMatrix = glm::mat4(1.0f);
	GLFWwindow* Window;

	irrklang::ISoundEngine* AudioEngine;

	Boat() {};

	void Update(float Delta) { // Update loop
		if (Window != NULL) { // check if window exists
			if (BoatMatrixPreWobble != glm::mat4(1.0f)) { // reset boat position before wobbling factor was applied, prevents issues
				BoatMatrix = BoatMatrixPreWobble;
			}

			float CalcSpeed = 1.0f * Delta; // get frame time calculation
			float InverseScale = 1.0f / BoatScale; // get boat scale inverse for calculations

			if (KeyPress(GLFW_KEY_W)) {
				BoatThrottle = Clamp(BoatThrottle + (AccelSpeed * CalcSpeed), 0.0f, 1.0f); // gradually raise speed 
			} else { 
				BoatThrottle = Clamp(BoatThrottle - (AccelSpeed * CalcSpeed * SlowDownMultiplier), 0.0f, 1.0f); // gradually lower speed
			}
			if (KeyPress(GLFW_KEY_A)) {
				BoatSteerLeft = Clamp(BoatSteerLeft + (SteerAccelSpeed * CalcSpeed), -1.0f, 0.0f); // gradually steer
				SteerWobbleLeft = Clamp(SteerWobbleLeft + (SteerWobbleAccel * CalcSpeed), -1.0f, 0.0f); // is two parts, was having less issues with this method
			} else {
				BoatSteerLeft = Clamp(BoatSteerLeft - (SteerAccelSpeed * CalcSpeed), -1.0f, 0.0f);
				SteerWobbleLeft = Clamp(SteerWobbleLeft - (SteerWobbleAccel * CalcSpeed), -1.0f, 0.0f);
			}
			if (KeyPress(GLFW_KEY_D)) {
				BoatSteerRight = Clamp(BoatSteerRight - (SteerAccelSpeed * CalcSpeed), 0.0f, 1.0f); // gradually steer
				SteerWobbleRight = Clamp(SteerWobbleRight - (SteerWobbleAccel * CalcSpeed), 0.0f, 1.0f);
			} else {
				BoatSteerRight = Clamp(BoatSteerRight + (SteerAccelSpeed * CalcSpeed), 0.0f, 1.0f);
				SteerWobbleRight = Clamp(SteerWobbleRight + (SteerWobbleAccel * CalcSpeed), 0.0f, 1.0f);
			}

			float FinalSteer = (BoatSteerLeft + BoatSteerRight) / 2; // calculate both steer inputs into one average

			if (CurrentTime() < LastBoostTime + (float(BoostDuration) * 1000)) { // boost speed detection
				BoatThrottle *= 1.5;
			}

			BoatMatrix = glm::rotate(BoatMatrix, glm::radians(FinalSteer * CalcSpeed * InverseScale * BoatThrottle * BoatSteerSpeed), glm::vec3(0.0f, 0.0f, 1.0f)); // Apply new transformations based on steer and speed
			BoatMatrix = glm::translate(BoatMatrix, glm::vec3(0.0f, BoatSpeed * CalcSpeed * InverseScale * BoatSpeed * BoatThrottle, 0.0f));
			BoatMatrixPreWobble = BoatMatrix;

			float FinalSteerWobble = (SteerWobbleLeft + SteerWobbleRight) / 2; // mix both wobbles into one like steering
			float CalcRotation = -FinalSteerWobble * SteerWobbleDegrees * BoatThrottle; // wobble based on steering amount

			LastWobble = glm::mix(LastWobble, CalcRotation, 0.01f * 144.0f * Delta); // storage for next loop's calculations before wobbling factor

			BoatMatrix = glm::rotate(BoatMatrix, glm::radians(LastWobble), glm::vec3(0.0f, 1.0f, 0.0f)); // final outcome
		}
	}

	void UpdateMouse(double X, double Y) {
		if (LastMouseX == -1 or LastMouseY == -1) { // sets new mouse origin locations to prevent random snaps on load
			LastMouseX = X;
			LastMouseY = Y;
			return;
		}

		float XChange = X - LastMouseX; // calculate change
		float YChange = Y - LastMouseY;

		XChange *= MouseSensitivity; // apply sensitivity factor
		YChange *= MouseSensitivity;

		Yaw = Clamp(Yaw + float(XChange),-60.0f,60.0f); // add new yaw to old yaw, clamping it to certain range
		Pitch = Clamp(Pitch + float(YChange), -10.0f, 20.0f); // same but for pitch

		LastMouseX = X; // mouse position for later calcs
		LastMouseY = Y;

		LastMoveTick = CurrentTime(); // last tick for panning camera back

	}

	void Init() { // initialisation, is reused to recenter boat now
		BoatMatrix = glm::mat4(1.0f);
		BoatMatrix = glm::scale(BoatMatrix, glm::vec3(BoatScale));
		BoatMatrix = glm::rotate(BoatMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		BoatMatrixPreWobble = BoatMatrix;
		BoatThrottle = 0.0f;
	}

	glm::mat4 GetBoatMatrix() { // was a readonly value at first so this is not required anymore
		return BoatMatrix;
	}

	glm::vec3 GetPositionFromMatrix(glm::mat4 Input) { // extracts position from mat4
		return glm::vec3(Input[3][0], Input[3][1], Input[3][2]);
	}
	
	glm::vec3 GetBoatPosition() {
		return GetPositionFromMatrix(BoatMatrix);
	}

	int GetBlurCycles() {
		if (CurrentTime() < LastPendingMove + 1000) {
			return 8;
		}
		return 0;
	}

	void SetPosition(glm::vec3 Input) { // sets boat mat4 position
		LastPendingMove = CurrentTime();
		BoatMatrix[3][0] = Input.x;
		BoatMatrix[3][2] = Input.z;
		BoatMatrixPreWobble[3][0] = Input.x;
		BoatMatrixPreWobble[3][2] = Input.z;
	}

	CameraData GetCameraData(float Delta) { // camera calculation loop
		float InverseScale = 1.0f / BoatScale; // get inverse of boat scale for reverting

		if (CurrentTime() > (RetractionDelay * 1000) + LastMoveTick) { // check time to see if camera should snap back
			Yaw = Lerp(Yaw, 0.0f, AimRetractionStrength * Delta); // smooth lerp movement
			//Pitch = Lerp(Pitch, 0.0f, AimRetractionStrength * Delta);
		}

		glm::mat4 NewMatrix = BoatMatrix; // new copy of boat matrix 
		NewMatrix = glm::rotate(NewMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // returns back to original state

		//NewMatrix = glm::rotate(NewMatrix, glm::radians(-Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
		NewMatrix = glm::rotate(NewMatrix, glm::radians(-Yaw), glm::vec3(0.0f, 1.0f, 0.0f));

		NewMatrix = glm::scale(NewMatrix, glm::vec3(InverseScale)); // return to normal size

		glm::vec3 BoatPosition = glm::vec3(NewMatrix[3]); // extract position from boat matrix
		glm::mat3 RotationMatrix = glm::mat3(NewMatrix); // extract rotation from boat matrix

		glm::vec3 CameraPosition = BoatPosition - (RotationMatrix * (glm::vec3(0.0f, -0.4f, -1.1f))); // get the camera position of the boat ensuring to move relative to the boat
		glm::vec3 CameraUp = RotationMatrix * glm::vec3(0.0f, 1.0f, 0.0f); // copy the boats up vector
		glm::vec3 CameraTarget = glm::vec3(BoatPosition.x, BoatPosition.y + 0.4f, BoatPosition.z); // where the camera aims at

		if (LastCameraPosition != glm::vec3(0.0f) && LastCameraOrientation != glm::vec3(0.0f)) { // checks to see if last values existed to interpolate with
			CameraPosition = glm::mix(LastCameraPosition, CameraPosition, (Smoothing * 144.0f) * Delta); // values were picked in 144hz testing so use it as a base
			CameraTarget = glm::mix(LastCameraOrientation, CameraTarget, (RotationSmoothing * 144.0f) * Delta); 
		}

		LastCameraPosition = CameraPosition; // update for next loop
		LastCameraOrientation = CameraTarget;
		glm::mat4 View = glm::lookAt(CameraPosition, CameraTarget, CameraUp); // final calculation

		return {CameraPosition, View}; // return the new struct
	}

	bool KeyPress(int Input) {
		if (glfwGetKey(Window, Input) == GLFW_PRESS) { // Ensuring that it has actually been pressed
			return true;
		}
		return false;
	}
};