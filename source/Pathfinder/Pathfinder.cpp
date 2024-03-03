#include "PathFinder.h"
#include <vector>
#include <map>
#include <queue>

using namespace std;

Pathfinder::Pathfinder() : filePath("D:\\UE5Projects\\UntilDawn\\NavData\\TestData.txt")
{
	InitializePathFinder();
}

void Pathfinder::InitializePathFinder()
{
	ifstream in(filePath);
	if (in.is_open()) 
	{
		in >> origin.X >> origin.Y >> origin.Z;
		in >> extent.X >> extent.Y >> extent.Z;
		in >> gridInterval >> lengthSize >> widthSize;
		grids = vector<vector<GridInfo>>(lengthSize, vector<GridInfo>(widthSize));
		for (int i = 0; i < lengthSize; i++)
		{
			for (int j = 0; j < widthSize; j++)
				in >> grids[i][j];
		}
	}
}

void Pathfinder::FindPath(const Vector3D& start, const Vector3D& dest, vector<Pos>& path)
{
	path.clear();

	const int dy = VectorToCoordinatesY(dest.Y);
	const int dx = VectorToCoordinatesX(dest.X);
	if (CanGo(dy, dx) == false)
	{
		// log
		return;
	}
	const int sy = VectorToCoordinatesY(start.Y);
	const int sx = VectorToCoordinatesX(start.X);

	// 목적지가 현재 위치 보다 높은 곳 인지? (높은 곳이면 true)
	const bool isGoingUp = grids[dy][dx].height - grids[sy][sx].height > 0 ? true : false;

	Pos startPos = Pos(sy, sx);
	vector<vector<bool>> visited(lengthSize, vector<bool>(widthSize, false));
	vector<vector<int>> best(lengthSize, vector<int>(widthSize, INT_MAX));
	map<Pos, Pos> parent;
	priority_queue<aStarNode> pq;

	Pos destPos(dy, dx);
	{
		int g = 0;
		int h = (abs(dy - sy) + abs(dx - sx)) * 10;
		pq.push({ -(g + h), g, startPos });
		best[sy][sx] = g + h;
		parent[startPos] = startPos;
	}

	bool findDest = false;
	Pos lastVistedPos;
	float minDest = 10000.f;
	while (!pq.empty())
	{
		aStarNode node = pq.top();
		pq.pop();

		if (visited[node.pos.y][node.pos.x] || best[node.pos.y][node.pos.x] < -node.f)
			continue;

		visited[node.pos.y][node.pos.x] = true;

		if (node.pos == destPos)
		{
			findDest = true;
			break;
		}

		for (int dir = 0; dir < 8; ++dir)
		{
			Pos nextPos = node.pos + front[dir];

			if (CanGo(nextPos.y, nextPos.x) == false || visited[nextPos.y][nextPos.x])
				continue;

			const int heightCost = floor(grids[node.pos.y][node.pos.x].height - grids[nextPos.y][nextPos.x].height + 0.5f) * (isGoingUp ? costWeight : -costWeight);
			// zombiePathCost 초기화 및 g에 점수 합산
			const int g = node.g + cost[dir] + grids[nextPos.y][nextPos.x].extraCost + heightCost;
			const int h = (abs(dy - nextPos.y) + abs(dx - nextPos.x)) * 10;
			if (best[nextPos.y][nextPos.x] <= g + h)
				continue;

			best[nextPos.y][nextPos.x] = g + h;
			pq.push({ -(g + h),g,nextPos });
			parent[nextPos] = node.pos;

			// 목적지에 도달 불가능한 경우를 대비해 목적지에 가장 가까운 그리드를 저장해둠
			if (destPos.GetDistance(nextPos) < minDest)
			{
				minDest = destPos.GetDistance(nextPos);
				lastVistedPos = nextPos;
			}
		}
	}

	Pos nextPos = findDest ? destPos : lastVistedPos;
	while (nextPos != parent[nextPos])
	{
		path.push_back(grids[nextPos.y][nextPos.x].pos);
		//gridIndexArr.push_back({nextPos.y, nextPos.x});
		//zombiePathCost[nextPos.y][nextPos.x] += 140;
		nextPos = parent[nextPos];
	}

	reverse(path.begin(), path.end());
}

bool Pathfinder::CanGo(const int y, const int x)
{
	if (y >= 0 && y < lengthSize && x >= 0 && x < widthSize)
	{
		if (grids[y][x].isPassable)
			return true;
	}
	return false;
}

int Pathfinder::VectorToCoordinatesY(const float vectorY)
{
	return floor(((vectorY - origin.Y + extent.Y) / gridInterval) + 0.5f);
}

int Pathfinder::VectorToCoordinatesX(const float vectorX)
{
	return floor(((vectorX - origin.X + extent.X) / gridInterval) + 0.5f);
}
