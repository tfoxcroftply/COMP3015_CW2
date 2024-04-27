
#include "helper/objmesh.h"
class Game {
private:
	std::unique_ptr<ObjMesh> NodeModel;
	int CurrentCheckpoint = 0;
	float DetectionDistance = 5;
	bool Finished = false;
	std::vector<glm::vec3> NodePositions = {
		glm::vec3(1,5,5),
		glm::vec3(2,5,5),
		glm::vec3(3,5,5)
	};
public:
	bool Init() {
		NodeModel = ObjMesh::load("resources/models/node.obj");
		if (NodeModel != nullptr) {
			return true;
		}
	}

	bool IsFinished() {
		return Finished;
	}

	std::vector<glm::vec3> GetActiveNodes() {
		if (CurrentCheckpoint < NodePositions.size()) {
			std::vector<glm::vec3> Temp;
			for (unsigned int i = CurrentCheckpoint; i < NodePositions.size(); i++) {
				Temp.push_back(NodePositions[i]);
			}
			return Temp;
		}
	}

	void UpdatePlayerPosition(glm::vec3 Input) {
		if (!Finished) {
			glm::vec3 NewPosition = glm::vec3(Input.x, 0, Input.z);
			glm::vec3 TargetNode = NodePositions[CurrentCheckpoint + 1];
			glm::vec3 TargetPosition = glm::vec3(TargetNode.x, 0, TargetNode.z);
			if (glm::distance(NewPosition, TargetPosition) < DetectionDistance) {
				std::cout << "Checkpoint update!";
				CurrentCheckpoint++;
			}
			if (CurrentCheckpoint == NodePositions.size()) {
				Finished = true;
			}
		}
	}
};

