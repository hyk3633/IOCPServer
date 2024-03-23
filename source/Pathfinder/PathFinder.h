#pragma once
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "../Structs/Pos.h"
#include "../Structs/Vector3D.h"

using namespace std;

#define NW normalWeight
#define HW heavyWeight

const int normalWeight = 30;
const int heavyWeight = 42;

struct GridInfo
{
	Pos pos;
	float height;
	bool isPassable;
	int extraCost;
	int pathCost = 0;

	friend ifstream& operator>>(ifstream& stream, GridInfo& grid)
	{
		stream >> grid.pos.x >> grid.pos.y >> grid.height >> grid.isPassable >> grid.extraCost;
		return stream;
	}
};

struct aStarNode
{
	int f;
	int g;
	Pos pos;

	aStarNode(int _f, int _g, Pos _pos) : f(_f), g(_g), pos(_pos) {}
	aStarNode() {}
	bool operator<(const aStarNode& other) const { return f < other.f; }
	bool operator>(const aStarNode& other) const { return f > other.f; }
};

class Pathfinder
{
public:

	Pathfinder();
	~Pathfinder() = default;

	static Pathfinder* GetPathfinder()
	{
		static Pathfinder pathfinder;
		return &pathfinder;
	}

	void FindPath(const Vector3D& start, const Vector3D& dest, vector<Pos>& path, vector<Pos>& pathIndexArr);

	void SetGridPassability(const Pos& pos, const bool isPassable);

	void ClearPathCost(const vector<Pos>& pathIndexArr);

protected:

	void InitializePathFinder();

	bool CanGo(const int y, const int x);

	int VectorToCoordinatesY(const float vectorY);
	int VectorToCoordinatesX(const float vectorX);

private:

	string filePath;

	int gridInterval;
	int lengthSize;
	int widthSize;

	Vector3D origin, extent;

	vector<vector<GridInfo>> grids;

	int obstacleCost = 12;
	int heightDifferenceLimit = 20;
	int costWeight = 5;

	Pos front[8] =
	{
		Pos { -1, 0},
		Pos { 0, -1},
		Pos { 1, 0},
		Pos { 0, 1},
		Pos {-1, -1},
		Pos {1, -1},
		Pos {1, 1},
		Pos {-1, 1}
	};

	int cost[8] = { NW,NW,NW,NW,HW,HW,HW,HW };
};