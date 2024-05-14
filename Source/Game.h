
#include "helper/objmesh.h"
#include "LevelData.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

std::string SavePath = "resources/saves/data.txt";

struct GameData {
	std::vector<long int> MillisecondsSorted;
	int TotalEntries = -1;
};



class Game {
private:
	float DetectionDistance = 0.5f;
	float NodeHeight = 0.25f;
	int MaxNodesVisible = 5;

	bool Finished = false;
	Boat boat;

	std::vector<Level> Levels;
	bool FirstLevel = true;

	glm::vec3 Convert(glm::vec2 Input) {
		return glm::vec3(Input.x, NodeHeight, Input.y);
	}

	std::vector<glm::vec3> ConvertVector(std::vector<glm::vec2> Input) {
		std::vector<glm::vec3> Collected;
		for (unsigned int i = 0; i < Input.size(); i++) {
			Collected.push_back(glm::vec3(Input[i].x, NodeHeight, Input[i].y));
		}
		return Collected;
	}

public:
	Game() {};
	std::unique_ptr<ObjMesh> NodeModel;
	unsigned int NodeTexture;
	unsigned int NodeNextTexture;
	unsigned int ActiveLevel = 0;
	std::string MenuOption = "Start";
	glm::vec3 RequestedBoatPos = glm::vec3(0.0f, 0.0f, 0.0f);

	bool Init() {
		std::vector<Level> NewLevels = GenerateLevels();

		for (unsigned int i = 0; i < NewLevels.size(); i++) {
			Levels.push_back(NewLevels[i]);
		}

		if (Levels.size() == 0) {
			std::cout << "No levels were generated.";
			return false;
		}

		NodeModel = ObjMesh::load("resources/models/node.obj");
		NodeTexture = LoadTexture("resources/textures/node.png");
		NodeNextTexture = LoadTexture("resources/textures/nodenext.png");

		LevelUpdate();

		if (NodeModel != nullptr && NodeTexture != -1) {
			return true;
		}

	}

	void LevelUpdate() {
		if (!Finished) {
			if (FirstLevel) {
				FirstLevel = false;
				return;
			}

			if (ActiveLevel + 1 < Levels.size()) {
				ActiveLevel++;
				std::cout << "Move boat";
				RequestedBoatPos = Convert(Levels[ActiveLevel].GetStart());
			} else {
				Finished = true;
			}


			if (ActiveLevel > 0) {
				MenuOption = "Running";
			}
		}
	}

	bool IsFinished() {
		return Finished;
	}

	std::vector<glm::vec3> GetActiveNodes() {
		if (!Finished) {
			if (ActiveLevel < Levels.size()) {
				std::vector<glm::vec2> NewNodes = Levels[ActiveLevel].GetActiveNodes();
				if (NewNodes.size() > 0) {
					return ConvertVector(NewNodes);
				}
			}
		}
		return {};
	}

	void UpdatePlayerPosition(glm::vec3 Input) {
		if (!Finished) {
			glm::vec2 NewPosition = glm::vec2(Input.x, Input.z);
			std::vector<glm::vec2> NewNodes = Levels[ActiveLevel].GetActiveNodes();
			if (NewNodes.size() > 0) {
				if (glm::distance(NewPosition, NewNodes[0]) < DetectionDistance) {
					Levels[ActiveLevel].CurrentCheckpoint++;
				}
			} else {
				LevelUpdate(); // backup
			}
		}
	}




	// data storage
	GameData LoadEntries() {
		std::ifstream File(SavePath);
		std::string Line;
		std::vector<long int> Collected;

		if (File.is_open()) {
			while (std::getline(File, Line)) {
				std::stringstream TempString(Line);

				int FoundNumber;
				if (TempString >> FoundNumber) {
					if (FoundNumber != 0) {
						Collected.push_back(FoundNumber);
						//std::cout << "Found time entry" << std::endl;
					} else {
						std::cout << "Time parse error." << std::endl;
					}
				} else {
					std::cout << "Time entry in invalid format." << std::endl;
					//std::cout << TempString.str() << std::endl;
				}
			}

			if (Collected.size() > 0) {
				std::sort(Collected.begin(), Collected.end());
				
				//for (unsigned int i = 0; i < Collected.size(); i++) {
					//std::cout << Collected[i] << std::endl;
				//}

				return { Collected, int(Collected.size()) };
			} else {
				std::cout << "No entries found." << std::endl;
				return { Collected, 0 };
			}
		} else {
			std::cout << "Failed to load leaderboard." << std::endl;
			return { Collected, -1 };
		}
		File.close();
	};

	bool SaveEntry(int Milliseconds) {
		std::ofstream File(SavePath, std::ios_base::app);
		if (File.is_open()) {
			if (File << std::endl << std::to_string(Milliseconds)) {
				std::cout << "Saved player data.";
				return true;
			} else {
				std::cout << "Failed to save player data." << std::endl;
			}
		} else {
			std::cout << "Failed to open save file." << std::endl;
		}
		File.close();
		return false;
	}
	
};

