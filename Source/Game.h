
#include "helper/objmesh.h"
class Game {
private:
	float DetectionDistance = 0.5f;
	float NodeHeight = 0.25f;
	int MaxNodesVisible = 5;
	int NodeMapScale = 3.0f;

	int CurrentCheckpoint = 0;
	bool Finished = false;
	std::vector<glm::vec2> NodePositionVectors = {};
	std::vector<float> NodePositions = {
		-1.0f,-1.0f,
		-2.0f,-2.0f,
		-3.0f,-5.0f,
		-6.0f,-5.0f,
		-6.0f,-2.0f,
		-4.0f,0.0f,
		-3.0f,3.0f,
		-1.0f,2.0f,
		-1.0f,-1.0f,
		1.0f,-3.0f,
		3.0f,-3.0f,
		5.0f,-2.0f,
		6.0f,-4.0f,
		4.0f,-6.0f,
		1.0f,-4.0f,
		5.0f,0.0f,
		5.0f,5.0f,
		2.0f,5.0f,
		-3.0f,7.0f,
		-5.0f,5.0f,
		-2.0f,4.0f,
		2.0f,3.0f,
		2.0f,1.0f,
		0.0f,0.0f
	};

public:
	std::unique_ptr<ObjMesh> NodeModel;
	unsigned int NodeTexture;
	unsigned int NodeNextTexture;

	bool Init() {

		if (NodePositions.size() % 2 != 0) {
			std::cout << "Node positions need to be in pairs.";
			return false;
		} else {
			for (unsigned int i = 0; i < NodePositions.size(); i += 2) {
				glm::vec2 NewNode = glm::vec2(NodePositions[i] * NodeMapScale, NodePositions[i + 1] * NodeMapScale);
				NodePositionVectors.push_back(NewNode);
			}
		}

		NodeModel = ObjMesh::load("resources/models/node.obj");
		NodeTexture = LoadTexture("resources/textures/node.png");
		NodeNextTexture = LoadTexture("resources/textures/nodenext.png");

		if (NodeModel != nullptr && NodeTexture != -1) {
			return true;
		}
	}

	bool IsFinished() {
		return Finished;
	}

	std::vector<glm::vec3> GetActiveNodes() {
		if (CurrentCheckpoint < NodePositionVectors.size()) {
			std::vector<glm::vec3> Temp;
			for (unsigned int i = CurrentCheckpoint; i < NodePositionVectors.size(); i++) {
				if (i >= CurrentCheckpoint + MaxNodesVisible) {
					break;
				}
				Temp.push_back(glm::vec3(NodePositionVectors[i].x,NodeHeight, NodePositionVectors[i].y));
			}
			return Temp;
		}
	}

	void UpdatePlayerPosition(glm::vec3 Input) {
		if (!Finished) {
			glm::vec2 NewPosition = glm::vec2(Input.x, Input.z);
			glm::vec2 SelectedNode = NodePositionVectors[CurrentCheckpoint];
			if (glm::distance(NewPosition, SelectedNode) < DetectionDistance) {
				std::cout << "Checkpoint update!";
				CurrentCheckpoint++;
			}
			if (CurrentCheckpoint == NodePositionVectors.size()) {
				Finished = true;
			}
		}
	}
};

