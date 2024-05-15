
#include "helper/objmesh.h"
#include "LevelData.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <irrKlang.h>

std::string SavePath = "resources/saves/data.txt";

struct GameData {
	std::vector<long int> MillisecondsSorted;
	int TotalEntries = -1;
};



class Game {
private:
	float DetectionDistance = 0.5f; // for detecting distance between boat and node
	float NodeHeight = 0.25f;
	int MaxNodesVisible = 5;
	int LevelCount = 0;

	GameData Cache;
	bool LoadedEntries = false;
	unsigned int MaxReturnEntries = 10;

	unsigned int StartTime = 0;

	bool Finished = false;
	Boat boat;

	std::vector<Level> Levels;
	bool FirstLevel = true;

	glm::vec3 Convert(glm::vec2 Input) {
		return glm::vec3(Input.x, NodeHeight, Input.y);
	}

	std::vector<glm::vec3> ConvertVector(std::vector<glm::vec2> Input) { // convert vec2 to vec3 using pre defined y value
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
	unsigned int NodeBoostTexture;
	unsigned int ActiveLevel = 0;
	unsigned int LastNodeCount = 0;
	unsigned int LastNodeTotal = 0;
	unsigned int LastBoostTime = 0;
	unsigned int SaveTime = 0;
	irrklang::ISoundEngine* AudioEngine;
	std::string MenuOption = "Start";
	glm::vec3 RequestedBoatPos = glm::vec3(0.0f, 0.0f, 0.0f);

	bool Init() {
		std::vector<Level> NewLevels = GenerateLevels(); // get levels list

		for (unsigned int i = 0; i < NewLevels.size(); i++) {
			Levels.push_back(NewLevels[i]); // save it locally
		}

		if (Levels.size() == 0) { // backup
			std::cout << "No levels were generated.";
			return false;
		}
		LevelCount = Levels.size() - 1; // excluding start level

		NodeModel = ObjMesh::load("resources/models/node.obj"); // load textures
		NodeTexture = LoadTexture("resources/textures/node.png");
		NodeNextTexture = LoadTexture("resources/textures/nodenext.png");
		NodeBoostTexture = LoadTexture("resources/textures/nodeboost.png");

		LevelUpdate(); // start level

		if (NodeModel != nullptr && NodeTexture != -1) { // checks to see if textures loaded properly
			return true;
		}

	}

	void Finish() { // parameter setting for finish detection
		Finished = true;
		MenuOption = "Finish";
		SaveTime = CurrentTime() - StartTime;
		SaveEntry(SaveTime); // save to file
	}

	void LevelUpdate() { // level updating logic, determines if game is finished or starting
		if (!Finished) {
			if (FirstLevel) {
				FirstLevel = false;
				return;
			}

			if (ActiveLevel + 1 < Levels.size()) {
				ActiveLevel++;
				LastNodeTotal = Levels[ActiveLevel].GetActiveNodes().size();
				RequestedBoatPos = Convert(Levels[ActiveLevel].GetStart());
			} else {
				Finish();
				return;
			}

			if (ActiveLevel > 0) {
				if (StartTime == 0) {
					StartTime = CurrentTime();
				}
				MenuOption = "Running";
			}
		}
	}

	int GetLevelCount() { // readonly returns
		return LevelCount;
	}

	bool IsFinished() {
		return Finished;
	}

	int GetTime() {
		return CurrentTime() - StartTime;
	}

	std::vector<glm::vec3> GetActiveNodes() { // get active nodes from level. this time it converts it to vec3
		if (!Finished) {
			if (ActiveLevel < Levels.size()) {
				std::vector<glm::vec2> NewNodes = Levels[ActiveLevel].GetActiveNodes();
				LastNodeCount = NewNodes.size();
				if (NewNodes.size() > 0) {
					return ConvertVector(NewNodes);
				}
			}
		}
		return {};
	}

	std::vector<glm::vec3> GetActiveBoosts() { // same as above but for boosts
		if (!Finished) {
			if (ActiveLevel < Levels.size()) {
				std::vector<glm::vec2> NewNodes = Levels[ActiveLevel].GetActiveBoosts();
				if (NewNodes.size() > 0) {
					return ConvertVector(NewNodes);
				}
			}
		}
		return {};
	}

	void UpdatePlayerPosition(glm::vec3 Input) { // update player position for checks
		if (!Finished) {
			glm::vec2 NewPosition = glm::vec2(Input.x, Input.z); // get position
			std::vector<glm::vec2> NewNodes = Levels[ActiveLevel].GetActiveNodes();
			std::vector<glm::vec2> NewBoosts = Levels[ActiveLevel].GetActiveBoosts();
			if (NewNodes.size() > 0) {
				if (glm::distance(NewPosition, NewNodes[0]) < DetectionDistance) { // for next node, check distance
					if (NewNodes.size() > 1) {
						AudioEngine->play2D("resources/sounds/collect.wav");
					} else {
						AudioEngine->play2D("resources/sounds/finish.wav");
					}
					Levels[ActiveLevel].CurrentCheckpoint++; // if threshold met, set checkpoint
				}
			} else {
				LevelUpdate(); // backup
			}
			if (NewBoosts.size() > 0) { // backup check, was having vector out of range issues
				for (unsigned int i = 0; i < NewBoosts.size(); i++) {
					if (glm::distance(NewPosition, NewBoosts[i]) < DetectionDistance) { // loop distance check for every visible boost, dont just check with the next one
						Levels[ActiveLevel].BoostCollect(NewBoosts[i]);
						AudioEngine->play2D("resources/sounds/boost.wav");
						LastBoostTime = CurrentTime(); // boost time tracking for speed incrase
					}
				}
			}
		}
	}




	// data storage
	GameData LoadEntries() { // load entry logic, only loads once and caches returned afterwards
		if (!LoadedEntries) {
			LoadedEntries = true;
			std::ifstream File(SavePath);
			std::string Line;
			std::vector<long int> Collected;

			if (File.is_open()) { // open file 
				while (std::getline(File, Line) and Collected.size() < MaxReturnEntries) { // read line by line
					std::stringstream TempString(Line);

					int FoundNumber;
					if (TempString >> FoundNumber) {
						if (FoundNumber != 0) {
							Collected.push_back(FoundNumber); // if valid then add it to collection
							//std::cout << "Found time entry" << std::endl;
						} else {
							std::cout << "Time parse error." << std::endl;
						}
					} else {
						std::cout << "Time entry in invalid format." << std::endl;
						//std::cout << TempString.str() << std::endl;
					}
				}

				if (Collected.size() > 0) { // check number collected
					std::sort(Collected.begin(), Collected.end());
				
					//for (unsigned int i = 0; i < Collected.size(); i++) {
						//std::cout << Collected[i] << std::endl;
					//}

					Cache = { Collected, int(Collected.size()) }; // set cache for later
					return Cache; // return
				} else {
					std::cout << "No entries found." << std::endl;
					return { Collected, 0 };
				}
			} else {
				std::cout << "Failed to load leaderboard." << std::endl;
				return { Collected, -1 };
			}
			File.close(); // always close to prevent issues
		}
		else {
			return Cache; // return cache instead of reloading file
		}
	};

	bool SaveEntry(int Milliseconds) { // save entry to file
		std::ofstream File(SavePath, std::ios_base::app);
		if (File.is_open()) {
			if (File << std::to_string(Milliseconds) << std::endl) {
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

