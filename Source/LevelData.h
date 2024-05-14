#pragma once

struct LevelDataStruct {
	glm::vec2 Start = glm::vec2(0.0f, 0.0f);
	std::vector<glm::vec2> Nodes;
	std::vector<glm::vec2> Boosts;
};

float NodeScale = 3.0f;

class Level {
private:
	unsigned int Id;
	glm::vec2 Start;
	std::vector<glm::vec2> Nodes;
	std::vector<glm::vec2> Boosts;
public:
	unsigned int CurrentCheckpoint = 0;
	Level() {};
	Level(unsigned int _Id, glm::vec2 _Start, std::vector<glm::vec2> _Nodes, std::vector<glm::vec2> _Boosts) {
		Id = _Id;
		Start = _Start;
		Nodes = _Nodes;
		Boosts = _Boosts;

		for (unsigned int i = 0; i < Nodes.size(); i++) {
			Nodes[i] *= NodeScale;
		}
		for (unsigned int i = 0; i < Boosts.size(); i++) {
			Boosts[i] *= NodeScale;
		}
	};

	unsigned int GetId() const {
		return Id;
	}

	std::vector<glm::vec2> GetActiveBoosts() {
		std::vector<glm::vec2> Collected = {};
		for (unsigned int i = 0; i < Boosts.size(); i++) {
			Collected.push_back(Boosts[i]);
		}
	}
	std::vector<glm::vec2> GetActiveNodes() {
		std::vector<glm::vec2> Collected = {};
		if (CurrentCheckpoint < Nodes.size()) {
			for (unsigned int i = CurrentCheckpoint; i < Nodes.size(); i++) {
				Collected.push_back(Nodes[i]);
			}
		}
		return Collected;
	}

	std::vector<glm::vec2> GetNextNode() {
		std::vector<glm::vec2> Collected; // so we can tell if it is empty
		if (Nodes.size() > 0) {
			Collected.push_back(Nodes[0]);
		}
		return Collected;
	}

	glm::vec2 GetStart() {
		return Start;
	}
};

std::vector<LevelDataStruct> LevelData = {
	{
		{0.0f,0.0f},
		{
			{1.0f,1.0f}
		}
	},
	{
		{-4.0f, -4.0f}, // start
		{
			{-4.0f, -2.0f}, // nodes
			{-3.0f, -1.0f},
			//{0.0f, 0.0f},
			//{2.0f, 1.0f},
			//{4.0f, 2.0f},
			//{4.0f, 4.0f} 
		},
		{
			{-1.0f, -0.5f} // boosts
		}
	}
};

std::vector<Level> GenerateLevels() {
	std::vector<Level> Collected;
	for (unsigned int i = 0; i < LevelData.size(); i++) {
		std::cout << "Detected new level" << std::endl;
		LevelDataStruct SelectedLevel = LevelData[i];
		Level New = Level(i,SelectedLevel.Start,SelectedLevel.Nodes,SelectedLevel.Boosts);
		std::cout << SelectedLevel.Nodes.size();
		Collected.push_back(New);
	}
	return Collected;
};

