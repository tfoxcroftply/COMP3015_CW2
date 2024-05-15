#pragma once

struct LevelDataStruct { // struct for passing batches of data around
	glm::vec2 Start = glm::vec2(0.0f, 0.0f);
	std::vector<glm::vec2> Nodes;
	std::vector<glm::vec2> Boosts;
};

float NodeScale = 1.5f; // scaling to space nodes further apart
glm::vec2 Offset = glm::vec2(0.0f, 5.0f);

class Level {
private:
	unsigned int Id;
	glm::vec2 Start;
	std::vector<glm::vec2> Nodes;
	std::vector<glm::vec2> Boosts;
	std::vector<glm::vec2> BoostBlacklist;
public:
	unsigned int CurrentCheckpoint = 0;
	Level() {};
	Level(unsigned int _Id, glm::vec2 _Start, std::vector<glm::vec2> _Nodes, std::vector<glm::vec2> _Boosts) {
		Id = _Id;
		Start = (_Start + Offset) * NodeScale; // scaling similar to below
		Nodes = _Nodes;
		Boosts = _Boosts;

		for (unsigned int i = 0; i < Nodes.size(); i++) { // save and scale nodes based on scale factor
			Nodes[i] = (Nodes[i] + Offset) * NodeScale;
		}
		for (unsigned int i = 0; i < Boosts.size(); i++) {
			Boosts[i] = (Boosts[i] + Offset) * NodeScale;
		}
	};

	unsigned int GetId() const { // return readonly id
		return Id;
	}

	void BoostCollect(glm::vec2 Input) { // blacklist adding for collected boosts
		BoostBlacklist.push_back(Input);
	}

	std::vector<glm::vec2> GetActiveBoosts() { // same with nodes below
		std::vector<glm::vec2> Collected = {};
		if (Boosts.size() > 0){
			for (unsigned int i = 0; i < Boosts.size(); i++) {
				if (!(std::find(BoostBlacklist.begin(), BoostBlacklist.end(), Boosts[i]) != BoostBlacklist.end())) { // check to see if node is in blacklist (aka collected list)
					Collected.push_back(Boosts[i]);
				}
			}
			return Collected;
		}

		return {};
	}
	std::vector<glm::vec2> GetActiveNodes() { // get all available nodes
		std::vector<glm::vec2> Collected = {};
		if (CurrentCheckpoint < Nodes.size()) {
			for (unsigned int i = CurrentCheckpoint; i < Nodes.size(); i++) { // only start from current checkpoint to not generate collected nodes
				Collected.push_back(Nodes[i]);
			}
		}
		return Collected;
	}

	std::vector<glm::vec2> GetNextNode() {
		std::vector<glm::vec2> Collected; // so we can tell if it is empty
		if (Nodes.size() > 0) {
			Collected.push_back(Nodes[0]); // send first node
		}
		return Collected;
	}

	glm::vec2 GetStart() { // pass read only value
		return Start;
	}
};

std::vector<LevelDataStruct> LevelData = { // level data definitions
	{
		{0.0f,0.0f}, // always used, no offset either
		{
			{0.0f,-8.0f}
		}
	},
	{ // level 1
		{0.0f, 15.0f}, // start
		{
			{-2.0f, 10.0f}, // nodes
			{-8.0f, 5.0f},
			{-10.0f, 0.0f},
			{-10.0f, -10.0f},
			{0.0f, -15.0f},
			{10.0f, -10.0f},
			{10.0f, 0.0f}
		},
		{
			{-8.0f, -5.0f}, // boosts
			{9.0f, -5.0f}
		}
	},
	{ // level 2
		{0.0f, 15.0f}, // start
		{
			{2.0f, 10.0f}, // nodes
			{5.0f, 5.0f},
			{10.0f, 0.0f},
			{10.0f, -5.0f},
			{5.0f, -10.0f},
			{0.0f, -10.0f},
			{-10.0f, -10.0f}
		},
		{
			{11.0f, -4.0f}, // boosts
			{-3.0f, -9.0f}
		}
	},
	{ // level 3
		{0.0f, 15.0f}, // start
		{
			{-2.0f, 10.0f}, // nodes
			{-2.0f, 3.0f},
			{4.0f, 0.0f},
			{8.0f, -2.0f},
			{10.0f, -6.0f},
			{7.0f, -10.0f},
			{0.0f, -10.0f}
		},
		{
			{1.0f, 2.0f}, // boosts
			{11.0f, -4.0f}
		}
	},
	{ // level 4
		{0.0f, 15.0f}, // start
		{
			{0.0f, 5.0f}, // nodes
			{0.0f, -5.0f},
			{5.0f, -10.0f},
			{12.0f, -10.0f},
			{16.0f, -5.0f},
			{13.0f, 2.0f},
			{4.0f, 2.0f},
			{-5.0f, 2.0f},
			{-13.0f, 2.0f}
		},
		{
			{-1.0f, 2.0f}, // boosts
			{1.0f, 2.0f}
		}
	},
	{ // level 5
		{0.0f, 15.0f}, // start
		{
			{-2.0f, 10.0f}, // nodes
			{-3.0f, 5.0f},
			{0.0f, 0.0f},
			{5.0f, -5.0f},
			{5.0f, -10.0f},
			{0.0f, -15.0f},
			{-5.0f, -15.0f},
			{-10.0f, -10.0f},
			{-10.0f, -5.0f}
		},
		{
			{-3.0f, 2.0f}, // boosts
			{-3.0f, -15.0f},
			{-7.0f,-14.0f},
			{-9.0f,-11.0f},
			{-10.0f,-9.0f},
			{-10.0f,-8.0f},
			{-10.0f,-6.0f},
		}
	},
};

std::vector<Level> GenerateLevels() { // level generation using level objects
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

