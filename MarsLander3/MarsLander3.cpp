#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <array>
#include <algorithm>
#include <random>
#include <map>
#include <set>
#include <chrono>

using namespace std;
static map<int, float> cosMap;
static map<int, float> sinMap;

struct Move
{
	int thrust;
	int angle;

	Move(int t, int a)
		: thrust(t)
		, angle(a)
	{}
};

struct SimResult
{
	Move firstMove;
	float score;

	SimResult(Move move, float score)
		: firstMove(move)
		, score(score)
	{
	}

	SimResult()
		: firstMove(Move(0, 0))
		, score(-100.0f)
	{}

};

struct SimCmp
{
	bool operator() (const SimResult& lhs, const SimResult& rhs)
	{
		return lhs.score > rhs.score;
	}
};

static queue<Move> moves;
// Going for a genetic approach


int orientation(const array<int, 2> &  p, const array<int, 2> &  q, const array<int, 2> &  r)
{
	int val = (q[0] - p[0]) * (r[1] - q[1]) -
		(q[1] - p[1]) * (r[0] - q[0]);

	if (val == 0) return 0;  // colinear

	return (val > 0) ? 1 : 2; // clock or counterclock wise
}

bool collision(const array<int, 2> & terrain1, const array<int, 2> & terrain2, const array<int, 2> & ship, const array<int, 2> & prevShip)
{
	// If slopes match, they don't collide
	// This is a simplification, expand if needed
	if (terrain2[1] == terrain1[1] || ship[1] == prevShip[1])
	{
	}


	else if ((terrain2[0] - terrain1[0]) / (terrain2[1] - terrain1[1]) == (ship[0] - prevShip[0]) / (ship[1] - prevShip[1]))
	{
		return false;
	}

	// Find the four orientations needed 
	int o1 = orientation(terrain1, terrain2, ship);
	int o2 = orientation(terrain1, terrain2, prevShip);
	int o3 = orientation(ship, prevShip, terrain1);
	int o4 = orientation(ship, prevShip, terrain2);


	if (o1 != o2 && o3 != o4) { return true; }

	return false;
}

struct Terrain
{
	vector<array<int, 2>> points;
	vector<int> segmentScores;
	int landSegment;
	int landingLeft;
	int landingRight;
	int landingHeight;
};

struct Ship
{
	int fuel, angle, thrust;
	float vSpeed, hSpeed;
	array<int, 2> pos;
	array<int, 2> startingPos;

	void advanceState()
	{
		// Given that the angle and thrust have already been updated, advance everything else for a 1 second timestep	
		float prevhSpeed = hSpeed;
		float prevvSpeed = vSpeed;
		hSpeed += thrust * sinMap[angle];
		vSpeed += thrust * cosMap[angle] - 3.711f;
		pos[0] += (vSpeed + prevvSpeed) / 2;
		pos[1] += (hSpeed + prevhSpeed) / 2;
		fuel -= thrust;
	}
};

struct Board
{
	const Terrain & terrain;
	Ship ship;
	static random_device rd;
	static mt19937 gen;
	static discrete_distribution<> dis;

	Board(Terrain& terrain)
		: terrain(terrain)
	{}

	SimResult randomSim()
	{
		// This simulates a game with random actions until it ends (ship collides with terrain)
		bool finished = false;
		bool first = true;
		int move = 0;
		Move firstMove(0, 0);

		int collisionSegment = -1;

		while (!finished)
		{
			++move;
			// Choose an action, start with angle
			// Favor angles closer to 0
			switch (ship.angle)
			{
			case -90:
				dis.param({ 1,19 });
				ship.angle += dis(gen) * 15;
				break;
			case -75:
				dis.param({ 1,5,25 });
				ship.angle += (dis(gen) - 1) * 15;
				break;
			case -60:
				dis.param({ 1,4,16 });
				ship.angle += (dis(gen) - 1) * 15;
				break;
			case -45:
				dis.param({ 1,3,9 });
				ship.angle += (dis(gen) - 1) * 15;
				break;
			case -30:
				dis.param({ 1,2.5,5 });
				ship.angle += (dis(gen) - 1) * 15;
				break;
			case -15:
				dis.param({ 1,2,4 });
				ship.angle += (dis(gen) - 1) * 15;
				break;
			case 0:
				dis.param({ 1,2,1 });
				ship.angle += (dis(gen) - 1) * 15;
				break;
			case 90:
				dis.param({ 1,19 });
				ship.angle -= dis(gen) * 15;
				break;
			case 75:
				dis.param({ 1,5,25 });
				ship.angle -= (dis(gen) - 1) * 15;
				break;
			case 60:
				dis.param({ 1,4,16 });
				ship.angle -= (dis(gen) - 1) * 15;
				break;
			case 45:
				dis.param({ 1,3,9 });
				ship.angle -= (dis(gen) - 1) * 15;
				break;
			case 30:
				dis.param({ 1,2.5,5 });
				ship.angle -= (dis(gen) - 1) * 15;
				break;
			case 15:
				dis.param({ 1,2,4 });
				ship.angle -= (dis(gen) - 1) * 15;
				break;
			default:
				break;
			}

			// Now choose thrust
			// Favor higher thrust
			if (ship.fuel <= 0) { ship.thrust = 0; }

			else
			{
				switch (ship.thrust)
				{
				case 0:
					dis.param({ 1,14 });
					ship.thrust += dis(gen);
					break;
				case 1:
					dis.param({ 1,7, 49});
					ship.thrust += dis(gen) - 1;
					break;
				case 2:
					dis.param({ 1,4,16 });
					ship.thrust += dis(gen) - 1;
					break;
				case 3:
					dis.param({ 1,2,4 });
					ship.thrust += dis(gen) - 1;
					break;
				case 4:
					dis.param({ 4,1 });
					ship.thrust -= dis(gen);
					break;
				default:
					break;
				}
			}

			if (first)
			{
				first = false;
				firstMove.angle = ship.angle;
				firstMove.thrust = ship.thrust;
			}

			// Advance the state of the ship
			array<int, 2> prevPos = ship.pos;
			ship.advanceState();

			// See if the game is over
			// Have I left the grid?
			if (ship.pos[0] < 0 || ship.pos[0] > 3000 || ship.pos[1] < 0 || ship.pos[1] > 7000)
			{
				finished = true;
			}

			// Has the ship collided with terrain?
			if (!finished)
			{
				for (int i = 1; i < terrain.points.size(); ++i)
				{
					if (collision(terrain.points[i - 1], terrain.points[i], ship.pos, prevPos))
					{
						// I should eventually check if I collided with the landing, since this is actually good
						finished = true;
						collisionSegment = i - 1;
						//cerr << "I will collide with segment " << terrain.points[i - 1][1] << " " << terrain.points[i - 1][0] << " " << terrain.points[i][1] << " " << terrain.points[i][0] << " at move " << move << endl;
						break;
					}
				}
			}
		}

		// Calculate the score of the position
		float score = calculateScore(collisionSegment);

		// Return the info needed
		return SimResult(firstMove, score);

	}

	float calculateScore(int collisionSegment) const
	{
		// Obviously, if I land successfully the score should be very high
		if (ship.pos[1] > terrain.landingLeft && ship.pos[1] < terrain.landingRight && ship.pos[0] - terrain.landingHeight < 50)
		{
			if (ship.angle == 0 && abs(ship.vSpeed) < 40.0f && abs(ship.hSpeed) < 20.0f)
			{
				return 999999999.0f;
			}
		}

		// The score is based on a few factors
		// crashing close to landing is good
		// high fuel is good
		float dist;
		if (collisionSegment < terrain.landSegment)
		{
			dist = abs(ship.pos[0] - terrain.points[collisionSegment + 1][0]) + abs(ship.pos[1] - terrain.points[collisionSegment + 1][1]);
		}

		else if (collisionSegment > terrain.landSegment)
		{
			dist = abs(ship.pos[0] - terrain.points[collisionSegment][0]) + abs(ship.pos[1] - terrain.points[collisionSegment][1]);
		}

		else
		{
			dist = abs(ship.pos[0] - terrain.landingHeight) + abs(ship.pos[1] - (terrain.landingLeft + terrain.landingRight) / 2);
		}

		return terrain.segmentScores[collisionSegment] * 6000.0f - dist + ship.fuel;
	}
};

random_device Board::rd;
mt19937 Board::gen(rd());
discrete_distribution<> Board::dis;


int main()
{
	cosMap[-90] = 0.0f;
	cosMap[-75] = 0.2588f;
	cosMap[-60] = 0.5f;
	cosMap[-45] = 0.7071f;
	cosMap[-30] = 0.866f;
	cosMap[-15] = 0.9659f;
	cosMap[0] = 1.0f;
	cosMap[90] = 0.0f;
	cosMap[75] = 0.2588f;
	cosMap[60] = 0.5f;
	cosMap[45] = 0.7071f;
	cosMap[30] = 0.866f;
	cosMap[15] = 0.9659f;

	sinMap[-90] = 1.0f;
	sinMap[-75] = 0.9659f;
	sinMap[-60] = 0.866f;
	sinMap[-45] = 0.7071f;
	sinMap[-30] = 0.5f;
	sinMap[-15] = 0.2588f;
	sinMap[0] = 0.0f;
	sinMap[90] = -1.0f;
	sinMap[75] = -0.9659f;
	sinMap[60] = -0.866f;
	sinMap[45] = -0.7071f;
	sinMap[30] = -0.5f;
	sinMap[15] = -0.2588f;

	set<SimResult, SimCmp> sortedResults;

	Terrain terrain;
	int surfaceN; // the number of points used to draw the surface of Mars.
	cin >> surfaceN; cin.ignore();
	int landSegment = -1;
	for (int i = 0; i < surfaceN; i++) {
		int landX; // X coordinate of a surface point. (0 to 6999)
		int landY; // Y coordinate of a surface point. By linking all the points together in a sequential fashion, you form the surface of Mars.
		cin >> landX >> landY; cin.ignore();

		terrain.points.push_back({ {landY, landX} });
		array<int, 2> & lastPoint = terrain.points[terrain.points.size() - 2];

		if (landY == lastPoint[0])
		{
			terrain.landingHeight = landY;
			terrain.landingLeft = min(landX, lastPoint[1]);
			terrain.landingRight = max(landX, lastPoint[1]);
			landSegment = i - 1;
			terrain.landSegment = i - 1;
		}
	}

	// Give scores to each segment
	for (int i = 0; i < landSegment; i++)
	{
		terrain.segmentScores.push_back(i + 10);
	}
	terrain.segmentScores.push_back(landSegment + 10);
	for (int i = landSegment + 1; i < surfaceN - 1; i++)
	{
		terrain.segmentScores.push_back(2 * landSegment - i + 10);
	}

	Board board(terrain);
	int X;
	int Y;
	int hSpeed; // the horizontal speed (in m/s), can be negative.
	int vSpeed; // the vertical speed (in m/s), can be negative.
	int fuel; // the quantity of remaining fuel in liters.
	int rotate; // the rotation angle in degrees (-90 to 90).
	int power; // the thrust power (0 to 4).

	bool first = true;
	SimResult bestResult;

	// game loop is 1 second long
	while (1) {

		auto start = chrono::high_resolution_clock::now();
		int sims = 0;
		sortedResults.clear();

		cin >> X >> Y >> hSpeed >> vSpeed >> fuel >> rotate >> power; cin.ignore();
		board.ship.angle = rotate;
		board.ship.pos[0] = Y;
		board.ship.pos[1] = X;
		board.ship.hSpeed = hSpeed;
		board.ship.vSpeed = vSpeed;
		board.ship.fuel = fuel;
		board.ship.thrust = power;

		if (first)
		{
			board.ship.startingPos[0] = Y;
			board.ship.startingPos[1] = X;
			first = false;
		}

		bestResult.score = -99999999.0f;

		while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() < 95)
		{
			++sims;

			// Start by simulating an entire run using random actions
			SimResult result = board.randomSim();
			//sortedResults.insert(result);

			if (result.score > bestResult.score)
			{
				bestResult = result;
			}

			//Reset the ship
			board.ship.angle = rotate;
			board.ship.pos[0] = Y;
			board.ship.pos[1] = X;
			board.ship.hSpeed = hSpeed;
			board.ship.vSpeed = vSpeed;
			board.ship.fuel = fuel;
			board.ship.thrust = power;
		}

		// Execute the best move
		cerr << sims << " sims" << endl;
		//SimResult best = *(sortedResults.begin());
		cout << bestResult.firstMove.angle << " " << bestResult.firstMove.thrust << endl;
	}
}