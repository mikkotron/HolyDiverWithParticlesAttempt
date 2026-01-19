/*
*
* Holy diver - an epic adventure at object-oriented world way beneath the surface!
* Template code for implementing the rich features to be specified later on.
*
*/

// NOTE: This project uses SFML (Graphics, Window) and requires linking against the SFML libraries.
// Tested with SFML3.0.0 and C++17

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <cmath>
#include <fstream>
#include <unordered_map>
#include <optional>
#include <sstream>



/****************************************************************
 *
 * ENUM InputAction
 *
 * Represents high-level player intentions parsed from input.
 * Used to control game flow (continue, abort, quit, etc.).
 *
 **************************************************************/
enum class InputAction {
	None,
	QuitGame,
	RestartLevel,
	AbortMission
};
/****************************************************************
 *
 * STRUCT HallOfFameEntry
 *
 * Represents a single saved player record.
 * Stored in a text file and loaded at runtime.
 *
 **************************************************************/
struct HallOfFameEntry {
	std::string name;
	int levelIndex;
	int treasuresCollected;
	int deaths;

	HallOfFameEntry(
		const std::string& n = "",
		int lvl = 0,
		int treas = 0,
		int d = 0.f)
		: name(n), levelIndex(lvl),
		treasuresCollected(treas), deaths(d) {
	}
};

/****************************************************************
 *
 * CLASS HallOfFame
 *
 * Handles loading, saving, appending, and displaying
 * persistent player statistics.
 *
 * Data format:
 * name,levelIndex,treasuresCollected,deaths
 *
 **************************************************************/
class HallOfFame {
public:
	std::vector<HallOfFameEntry> entries;
	std::string filename = "hall_of_fame.txt";

	// Load all entries from file
	void load() {
		entries.clear();
		std::ifstream file(filename);
		if (!file.is_open()) return;

		std::string line;
		while (std::getline(file, line)) {
			std::istringstream iss(line);

			std::string name;
			std::string levelStr, treasuresStr, deathsStr;

			if (std::getline(iss, name, ',') &&
				std::getline(iss, levelStr, ',') &&
				std::getline(iss, treasuresStr, ',') &&
				std::getline(iss, deathsStr, ',')) {

				entries.push_back({
					name,
					std::stoi(levelStr),
					std::stoi(treasuresStr),
					std::stoi(deathsStr)
					});
			}
		}
	}

	// Save all entries (overwrite)
	void save() {
		std::ofstream file(filename);
		if (!file.is_open()) return;

		for (auto& entry : entries) {
			file << entry.name << "," << entry.levelIndex << ","
				<< entry.treasuresCollected << "," << entry.deaths << "\n";
		}
	}

	// Add a single entry and append it to file
	void addEntry(const HallOfFameEntry& entry) {
		entries.push_back(entry);

		std::ofstream file(filename, std::ios::app);
		if (!file.is_open()) return;

		file << entry.name << "," << entry.levelIndex << ","
			<< entry.treasuresCollected << "," << entry.deaths << "\n";
	}

	// Print leaderboard to console
	void display() {
		std::cout << "\n=== Hall of Fame Workers of The Month ===\n";
		for (auto& e : entries) {
			std::cout << e.name
				<< " | Level " << e.levelIndex
				<< " | Minerals: " << e.treasuresCollected
				<< " | Deaths: " << e.deaths << "\n";
		}
		std::cout << "===========================\n";
	}
};

// solves collisions with boundaries of the map and applys movement and visual style for the particle
struct Particle {
	sf::Vector2f position;
	sf::Vector2f position_last;
	sf::Vector2f acceleration;
	float radius = 6.0f;
	sf::CircleShape shape;

	void solveBoundaries(const sf::RectangleShape& rect) {
		float r = shape.getRadius();

		float outlineHalf = rect.getOutlineThickness() / 2.f;
		float outline = rect.getOutlineThickness();

		float left = rect.getPosition().x - rect.getSize().x / 2.f - outline;
		float right = rect.getPosition().x + rect.getSize().x / 2.f + outlineHalf;
		float top = rect.getPosition().y - rect.getSize().y / 2.f - outline;
		float bottom = rect.getPosition().y + rect.getSize().y / 2.f + outlineHalf;

		float b = 0.95f; // bounce factor

		// LEFT boundary
		if (position.x - r < left) {
			position.x = left + r;
			position_last.x = position.x + (position_last.x - position.x) * -b;
		}
		// RIGHT boundary
		if (position.x + r > right) {
			position.x = right - r;
			position_last.x = position.x + (position_last.x - position.x) * -b;
		}
		// TOP boundary
		if (position.y - r < top) {
			position.y = top + r;
			position_last.y = position.y + (position_last.y - position.y) * -b;
		}
		// BOTTOM boundary
		if (position.y + r > bottom) {
			position.y = bottom - r;
			position_last.y = position.y + (position_last.y - position.y) * -b;
		}
	}

	Particle() = default;
	Particle(sf::Vector2f position_, float radius_)
		: position{ position_ }, position_last{ position_ }, acceleration{ 0.f, 0.f }, radius{ radius_ }
	{
		shape.setRadius(radius);
		shape.setFillColor(sf::Color(0, 150, 255, 255));
		shape.setPosition(position);
	}

	void update(float dt) {
		float damping = 0.99f;
		sf::Vector2f displacement = (position - position_last) * damping;
		position_last = position;
		position = position + displacement + acceleration * (dt * dt);
		acceleration = {};
		shape.setPosition(position);
	}

	void applyAcceleration(sf::Vector2f a) { acceleration += a; }
	void setVelocity(sf::Vector2f v, float dt) { position_last = position - (v * dt); }
	void addVelocity(sf::Vector2f v, float dt) { position_last -= v * dt; }
	sf::Vector2f getVelocity() { return position - position_last; }
};


/// <summary>
///  Uses grid hashing to check collision, instead of checking the hole screen for collisions with every other particle each frame it only checks the ones that are in side of the same hash for the particle
/// </summary>
class Solver {
public:
	struct SpatialHashGrid {
		float cellSize;
		std::unordered_map<long long, std::vector<int>> grid;
		SpatialHashGrid(float cellSize_) : cellSize(cellSize_) {}
		long long hash(int x, int y) const { return ((long long)x << 32) ^ (long long)(y); }
		sf::Vector2i worldToCell(const sf::Vector2f& pos, const sf::Vector2f& rectTopLeft) const {
			sf::Vector2f relativePos = pos - rectTopLeft;
			return sf::Vector2i((int)std::floor(relativePos.x / cellSize), (int)std::floor(relativePos.y / cellSize));
		}
		void insert(const sf::Vector2f& pos, int index, const sf::Vector2f& rectTopLeft) {
			sf::Vector2i cell = worldToCell(pos, rectTopLeft);
			long long key = hash(cell.x, cell.y);
			grid[key].push_back(index);
		}
		void clear() { grid.clear(); }
	};

	Solver() = default;
	SpatialHashGrid hashGrid{ 45.0f };
	Particle& addObject(sf::Vector2f position, float radius) { Particle newParticle(position, radius); return objects.emplace_back(newParticle); }

	void update(const sf::RectangleShape& rect) {
		applyGravity();
		sf::Vector2f rectTopLeft = rect.getPosition() - rect.getSize() / 2.f;
		hashGrid.clear();
		for (int i = 0; i < objects.size(); ++i) hashGrid.insert(objects[i].position, i, rectTopLeft);

		for (int i = 0; i < 3; ++i) {
			for (auto& obj : objects) { obj.solveBoundaries(rect); obj.update(step_dt); }
			checkCollisionsSpatial(rect);
		}
	}

	void drawGrid(const sf::RectangleShape& rect, sf::RenderWindow& window) {
		float cellSize = hashGrid.cellSize;
		sf::RectangleShape cellOutline;
		cellOutline.setSize(sf::Vector2f(cellSize, cellSize));
		cellOutline.setFillColor(sf::Color::Transparent);
		cellOutline.setOutlineColor(sf::Color(60, 60, 60, 120));
		cellOutline.setOutlineThickness(1.f);

		sf::Vector2f rectTopLeft = rect.getPosition() - rect.getSize() / 2.f;
		int startCol = 0;
		int endCol = static_cast<int>(std::ceil(rect.getSize().x / cellSize));
		int startRow = 0;
		int endRow = static_cast<int>(std::ceil(rect.getSize().y / cellSize));

		for (int x = startCol; x < endCol; x++) {
			for (int y = startRow; y < endRow; y++) {
				cellOutline.setPosition(rectTopLeft + sf::Vector2f(x * cellSize, y * cellSize));
				window.draw(cellOutline);
			}
		}
	}

	std::vector<Particle>& getObjects() { return objects; }

private:
	std::vector<Particle> objects;
	sf::Vector2f gravity = { 0.0f, 800 };
	float step_dt = 1.0f / 120;

	void applyGravity() { for (auto& obj : objects) obj.applyAcceleration(gravity); }

	void checkCollisionsSpatial(const sf::RectangleShape& rect) {
		sf::Vector2f rectTopLeft = rect.getPosition() - rect.getSize() / 2.f;
		sf::Vector2f rectBottomRight = rect.getPosition() + rect.getSize() / 2.f;

		for (auto& [key, cellParticles] : hashGrid.grid) {
			int cx = static_cast<int>(key >> 32);
			int cy = static_cast<int>(key & 0xFFFFFFFF);

			float cellLeft = rectTopLeft.x + cx * hashGrid.cellSize;
			float cellRight = cellLeft + hashGrid.cellSize;
			float cellTop = rectTopLeft.y + cy * hashGrid.cellSize;
			float cellBottom = cellTop + hashGrid.cellSize;

			if (cellRight < rectTopLeft.x || cellLeft > rectBottomRight.x || cellBottom < rectTopLeft.y || cellTop > rectBottomRight.y) continue;

			for (int dx = -1; dx <= 1; ++dx) {
				for (int dy = -1; dy <= 1; ++dy) {
					long long neighborKey = hashGrid.hash(cx + dx, cy + dy);
					auto it = hashGrid.grid.find(neighborKey);
					if (it == hashGrid.grid.end()) continue;

					const auto& neighborParticles = it->second;
					for (int i : cellParticles) {
						Particle& obj1 = objects[i];
						for (int j : neighborParticles) {
							if (j <= i) continue;
							Particle& obj2 = objects[j];
							sf::Vector2f v = obj1.position - obj2.position;
							float dist = sqrt(v.x * v.x + v.y * v.y);
							float min_dist = obj1.radius + obj2.radius;
							if (dist < min_dist) {
								sf::Vector2f n = dist > 0.0001f ? v / dist : sf::Vector2f(1.f, 0.f);
								float delta = 0.8f * (min_dist - dist);
								obj1.position += n * 0.5f * delta;
								obj2.position -= n * 0.5f * delta;
								float damping = 0.99f;
								obj1.position_last = obj1.position - (obj1.position - obj1.position_last) * damping;
								obj2.position_last = obj2.position - (obj2.position - obj2.position_last) * damping;
							}
						}
					}
				}
			}
		}
	}
};


// ------------------- Wall -------------------
// Simple static wall object used for collision and rendering
class Wall {
public:
	sf::RectangleShape shape;

	// Construct a wall with position and size
	Wall(float x, float y, float width, float height) {
		shape.setSize({ width, height });
		shape.setFillColor(sf::Color::Blue);
		shape.setPosition({ x, y });
	}

	// Draw wall to render window
	void draw(sf::RenderWindow& window) {
		window.draw(shape);
	}
};

// ------------------- Globals -------------------
// Toggle for particle simulation mode
bool openParticleSim = false;

// Bounding rectangle for gameplay area
sf::RectangleShape rectangle;

// Particle physics solver
Solver particleSolver;

// Map dimensions
int rows = 0;
int cols = 0;

// Tile map data
char** map = nullptr;

// Collection of walls for cave collision detection for the player
std::vector<class Wall> walls;

// ------------------- Entity / Player / Enemy -------------------
// Base class for all entities that have health and position also handles the boundarie collisions
class Entity {
public:
	int health = 100;
	int x = 0, y = 0;

	// Clamp an object inside a rectangular boundary
	sf::Vector2f clampInsideRect(
		const sf::Vector2f& pos,
		const sf::Vector2f& size,
		const sf::RectangleShape& bounds
	) {
		sf::Vector2f topLeft = bounds.getPosition() - bounds.getSize() / 2.f;
		sf::Vector2f bottomRight = bounds.getPosition() + bounds.getSize() / 2.f;
		sf::Vector2f halfSize = size / 2.f;
		sf::Vector2f clamped = pos;

		if (clamped.x - halfSize.x < topLeft.x) clamped.x = topLeft.x + halfSize.x;
		if (clamped.x + halfSize.x > bottomRight.x) clamped.x = bottomRight.x - halfSize.x;
		if (clamped.y - halfSize.y < topLeft.y) clamped.y = topLeft.y + halfSize.y;
		if (clamped.y + halfSize.y > bottomRight.y) clamped.y = bottomRight.y - halfSize.y;

		return clamped;
	}
};

// ------------------- Player -------------------
// Player-controlled entity
class Player : public Entity {
public:
	float oxygenTime = 30.0f;        // Remaining oxygen
	int deaths = 0;                 // Death counter
	int totalTreasuresCollected = 0;

	// Create player with given size
	Player(int x = 0, int y = 0, float sizeX = 20.f, float sizeY = 20.f) {
		rect.setSize(sf::Vector2f(sizeX, sizeY));
		up = down = left = right = false;
	}

	// Set player position
	void setPosition(const sf::Vector2f& pos) {
		rect.setPosition(pos);
	}

	// Set player color
	void setFillColor(const sf::Color& color) {
		rect.setFillColor(color);
	}

	// Handle keyboard input
	void ProcessEvents(sf::Keyboard::Key key, bool checkPressed) {
		switch (key) {
		case sf::Keyboard::Key::W: up = checkPressed; break;
		case sf::Keyboard::Key::S: down = checkPressed; break;
		case sf::Keyboard::Key::A: left = checkPressed; break;
		case sf::Keyboard::Key::D: right = checkPressed; break;
		}
	}

	// Update player movement and collisions
	void update(const std::vector<class Wall>& walls) {
		sf::Vector2f movement;
		if (up) movement.y -= 2.f;
		if (down) movement.y += 2.f;
		if (left) movement.x -= 2.f;
		if (right) movement.x += 2.f;

		rect.move(movement);
		rect.setPosition(clampInsideRect(rect.getPosition(), rect.getSize(), rectangle));
		resolveCollisions(walls);
	}

	// Draw player
	void drawTo(sf::RenderWindow& window) {
		window.draw(rect);
	}

	// Axis-aligned bounding box collision check
	bool checkCollision(const sf::RectangleShape& wallShape) {
		sf::Vector2f playerPos = rect.getPosition();
		sf::Vector2f playerHalf = rect.getSize() / 2.f;
		sf::Vector2f wallPos = wallShape.getPosition();
		sf::Vector2f wallHalf = wallShape.getSize() / 2.f;

		return (abs(playerPos.x - wallPos.x) < (playerHalf.x + wallHalf.x)) &&
			(abs(playerPos.y - wallPos.y) < (playerHalf.y + wallHalf.y));
	}

	// Resolve collisions with walls by separating overlapping axes
	void resolveCollisions(const std::vector<class Wall>& walls) {
		sf::Vector2f playerPos = rect.getPosition();
		sf::Vector2f playerHalf = rect.getSize() / 2.f;

		for (auto& wall : walls) {
			sf::Vector2f wallPos = wall.shape.getPosition();
			sf::Vector2f wallHalf = wall.shape.getSize() / 2.f;

			if (checkCollision(wall.shape)) {
				sf::Vector2f delta = playerPos - wallPos;
				float overlapX = (playerHalf.x + wallHalf.x) - abs(delta.x);
				float overlapY = (playerHalf.y + wallHalf.y) - abs(delta.y);

				if (overlapX < overlapY)
					playerPos.x += delta.x > 0 ? overlapX : -overlapX;
				else
					playerPos.y += delta.y > 0 ? overlapY : -overlapY;

				rect.setPosition(playerPos);
			}
		}
	}

	// Get current position
	sf::Vector2f getPosition() const {
		return rect.getPosition();
	}

	// Apply damage and handle death
	void takeDamage(int damage, sf::RenderWindow& window) {
		health -= damage;
		if (health < 0) health = 0;

		if (health == 0) {
			std::cout << std::endl << "PLAYER DIED!\n";
			deaths++;
			window.close();
		}
	}

	// Check if player is dead
	bool isDead() const {
		return health <= 0;
	}

	// Reset player state
	void reset() {
		health = 100;
		oxygenTime = 30.0f;
		up = down = left = right = false;
	}

private:
	sf::RectangleShape rect;
	bool up, down, left, right;
};

// Global player instance
Player playa(0, 0, 20.f, 20.f);


// ------------------- Enemy -------------------
// Enemy entity with two movement behaviors
class Enemy : public Entity {
public:
	enum class Type { Moving, Oscillating };
	Type type = Type::Moving;

	std::vector<Enemy> enemies;
	int id;

	sf::Vector2f pos;
	float speed = 70.f;
	sf::RectangleShape shape;

	float damageCooldown = 1.0f;
	sf::Clock damageClock;

	sf::Vector2f randomOffset;

	// Oscillation parameters
	float oscillationAmplitude = 50.f;
	float oscillationSpeed = 2.f;
	sf::Vector2f startPos;

	// Construct enemy
	Enemy(int x, int y, int id_, Type t = Type::Moving) : id(id_), type(t) {
		pos = sf::Vector2f((float)x, (float)y);
		startPos = pos;

		shape.setSize({ 20.f, 20.f });
		shape.setFillColor(type == Type::Moving ? sf::Color::Red : sf::Color::Blue);
		shape.setPosition(pos);

		// Random offset for moving enemies
		if (type == Type::Moving) {
			float maxOffset = 10.f;
			randomOffset = sf::Vector2f(
				static_cast<float>((rand() % (int)maxOffset) - maxOffset / 2),
				static_cast<float>((rand() % (int)maxOffset) - maxOffset / 2)
			);
		}
	}

	// Update enemy behavior and damage player if close
	void update(float dt, const sf::Vector2f& playerPos, sf::RenderWindow& window, Player& playa) {
		static sf::Clock clock;
		float time = clock.getElapsedTime().asSeconds();

		if (type == Type::Oscillating) {
			pos.y = startPos.y + std::sin(time * oscillationSpeed) * oscillationAmplitude;
			shape.setPosition(pos);
		}
		else {
			float wiggleStrength = 80.f;
			sf::Vector2f noiseOffset(
				std::sin(time + id) * wiggleStrength,
				std::cos(time + id) * wiggleStrength
			);

			sf::Vector2f target = playerPos + noiseOffset;
			sf::Vector2f dir = target - pos;
			float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (length > 0.f) dir /= length;

			pos += dir * speed * dt;
			pos = clampInsideRect(pos, shape.getSize(), rectangle);
			shape.setPosition(pos);
		}

		float distance = std::sqrt(
			std::pow(playerPos.x - pos.x, 2.f) +
			std::pow(playerPos.y - pos.y, 2.f)
		);

		if (distance <= 30.f && damageClock.getElapsedTime().asSeconds() >= damageCooldown) {
			playa.takeDamage(5, window);
			damageClock.restart();
		}
	}

	// Draw enemy
	void draw(sf::RenderWindow& window) {
		window.draw(shape);
	}
};
// ------------------- Items -------------------
// Base class for collectible items
class Item {
public:
	sf::Vector2f position;
	sf::RectangleShape shape;
	bool collected = false;

	Item(sf::Vector2f pos, sf::Color color, float size = 15.f) : position(pos) {
		shape.setSize({ size, size });
		shape.setFillColor(color);
		shape.setPosition(position);
	}

	virtual void applyEffect(Player& player) = 0;
};

// Health-restoring item
class HydraMineral : public Item {
public:
	HydraMineral(sf::Vector2f pos) : Item(pos, sf::Color::Yellow) {}
	void applyEffect(Player& player) override {
		player.health = std::min(player.health + 5, 100);
	}
};

// Oxygen-restoring item
class Oxygen : public Item {
public:
	Oxygen(sf::Vector2f pos) : Item(pos, sf::Color::White) {}
	void applyEffect(Player& player) override {
		player.oxygenTime = std::min(player.oxygenTime + 10.f, 30.f);
	}
};

// ------------------- Level -------------------
// Handles level data, maps, and collectibles
class Level {
public:
	bool customMapFile = false;
	std::string customMapFileName;

	std::vector<Item*> items;
	std::vector<std::string> mapFiles = { "lvl1.txt", "lvl2.txt", "lvl3.txt" };
	int currentMapIndex = 0;
	bool requestCloseRender = false;

	int getTotalTreasures() const {
		return static_cast<int>(items.size());
	}

	int getCollectedTreasures() const {
		int count = 0;
		for (auto& item : items)
			if (item->collected) count++;
		return count;
	}

	void resetCollectedTreasures() {
		for (auto& item : items)
			item->collected = false;
	}

	// Access player's total treasure count
	int& addCollectedToTotal(Player& playa) {
		return playa.totalTreasuresCollected;
	}
};

// ------------------- Function declarations -------------------
void start_splash_screen(Level& currentLevel);
void startup_routines(Level& currentLevel, Enemy& enemies);
void quit_routines(Level& currentLevel);
void load_level(Level& currentlevel, Enemy& enemy);
InputAction read_input(char*, Level& currentLevel);
void handleAbortMission(Level& currentLevel, Enemy& enemy);
void render_screen(Level& currentLevel, Enemy& enemy);
void resetMap(Level& currentLevel, Enemy& enemy);

// ------------------- Main -------------------
char input;
/****************************************************************
 *
 * MAIN
 * main function contains merely function calls to various routines and the main game loop
 *
 ****************************************************/
int main(void)
{
	Level currentLevel;
	Enemy enemy(200, 200, 0);

	// Show splash screen and instructions
	start_splash_screen(currentLevel);

	// Initial game setup (load level, spawn objects, etc.)
	startup_routines(currentLevel, enemy);

	// IMPORTANT NOTE: do not exit program without cleanup
	while (true) {
		// Read player input (console-based)
		InputAction action = read_input(&input, currentLevel);

		// Handle input actions
		switch (action) {
		case InputAction::QuitGame:
			quit_routines(currentLevel);
			return 0;

		case InputAction::RestartLevel:
			resetMap(currentLevel, enemy);
			break;

		case InputAction::AbortMission:
			handleAbortMission(currentLevel, enemy);
			break;

		case InputAction::None:
			break;
		}

		// Launch SFML rendering loop when allowed
		if (openParticleSim)
			render_screen(currentLevel, enemy);
	}

	// Safety cleanup (normally unreachable)
	quit_routines(currentLevel);
	return 0;
}


 /****************************************************************
  * FUNCTION load_level
  * Loads a map file and initializes all map-related data.
  ****************************************************************/
void load_level(Level& currentLevel, Enemy& enemy)
{
	// Reset player and collected treasures
	playa.reset();
	currentLevel.resetCollectedTreasures();

	// Free old map memory
	if (map != nullptr) {
		for (int i = 0; i < rows; ++i)
			delete[] map[i];
		delete[] map;
		map = nullptr;
	}

	// Delete old items
	for (auto& item : currentLevel.items)
		delete item;
	currentLevel.items.clear();

	// Clear enemies and walls
	enemy.enemies.clear();
	walls.clear();

	rows = 0;
	cols = 0;

	// Determine map filename
	std::string filename;
	if (currentLevel.customMapFile) {
		filename = currentLevel.customMapFileName;
	}
	else {
		if (currentLevel.currentMapIndex < 0 ||
			currentLevel.currentMapIndex >= currentLevel.mapFiles.size()) {
			std::cerr << "Error: invalid map index\n";
			return;
		}
		filename = currentLevel.mapFiles[currentLevel.currentMapIndex];
	}

	// Open map file
	std::ifstream file(filename);
	if (!file) {
		std::cerr << "Failed to open map file: " << filename << std::endl;
		return;
	}

	// Read file contents line by line
	std::vector<std::string> contents;
	std::string line;
	while (getline(file, line))
		contents.push_back(line);

	if (contents.empty()) {
		std::cerr << "Error: empty map file\n";
		return;
	}

	// Allocate new map
	rows = static_cast<int>(contents.size());
	cols = static_cast<int>(contents[0].size());

	map = new char* [rows];
	for (int i = 0; i < rows; i++) {
		map[i] = new char[cols];
		for (int j = 0; j < cols; j++)
			map[i][j] = contents[i][j];
	}
}

/****************************************************************
 *
 * FUNCTION read_input
 *
 * Reads console input from the user and converts it into a game action.
 * If the player is dead, the mission is automatically aborted.
 *
 **************************************************************/
InputAction read_input(char* input, Level& currentLevel)
{
	// If player is alive, show available commands
	if (!playa.isDead())
		std::cout << "1 = continue | a = abort mission | q = quit\n";
	else
		// Dead player cannot continue, force abort
		return InputAction::AbortMission;

	// Read single-character input
	std::cout << ">>> ";
	std::cin >> *input;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	// Translate input character into an InputAction enum
	switch (*input) {
	case 'q':
		return InputAction::QuitGame;
	case '1':
		openParticleSim = true;   // allow rendering loop to start
		return InputAction::None;
	case 'a':
		return InputAction::AbortMission;
	default:
		// Any unknown input does nothing
		return InputAction::None;
	}
}

/****************************************************************
 *
 * FUNCTION handleAbortMission
 *
 * Handles player choices when a mission is aborted or the player dies.
 * Allows restarting, moving between maps, or quitting to menu.
 *
 **************************************************************/
void handleAbortMission(Level& currentLevel, Enemy& enemy)
{
	// Count collected and total treasures
	int collected = currentLevel.getCollectedTreasures();
	int total = static_cast<int>(currentLevel.items.size());

	// Percentage of collected items
	float percentage = total > 0 ? (float)collected / total : 0.f;

	// Reward for aborting (half of total items)
	int endResult = total / 2;

	// Check if advancing to next level is allowed
	bool canAdvance =
		percentage >= 0.5f &&
		currentLevel.currentMapIndex < currentLevel.mapFiles.size() - 1 &&
		!playa.isDead();

	if (!playa.isDead()) {
		// Add partial progress to total score
		currentLevel.addCollectedToTotal(playa) += endResult;

		// Show mission summary and options
		std::cout << "\nMISSION ABORTED!\n";
		std::cout << "You collected " << collected << " out of " << total << " treasures.\n";
		std::cout << "Choose an option:\n";
		std::cout << "r = restart current map\n";
		std::cout << "p = previous map (if any)\n";
		if (canAdvance) std::cout << "n = next map\n";
		std::cout << "q = quit to menu\n>>> ";
	}
	else {
		// If player died, reset player state and show limited options
		playa.reset();
		std::cout << "Choose an option:\n";
		std::cout << "r = restart current map\n";
		std::cout << "p = previous map (if any)\n";
		std::cout << "q = quit to menu\n>>> ";
	}

	// Loop until valid menu choice is made
	while (true) {
		char choice;
		std::cin >> choice;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		if (choice == 'r') {
			// Restart the current level
			resetMap(currentLevel, enemy);
			break;
		}
		else if (choice == 'p' && currentLevel.currentMapIndex > 0) {
			// Go back to previous level
			currentLevel.customMapFile = false;
			currentLevel.currentMapIndex--;
			resetMap(currentLevel, enemy);
			break;
		}
		else if (choice == 'n' && canAdvance) {
			// Advance to next level
			currentLevel.customMapFile = false;
			currentLevel.currentMapIndex++;
			resetMap(currentLevel, enemy);
			break;
		}
		else if (choice == 'q') {
			// Quit to menu (stop rendering loop)
			openParticleSim = false;
			resetMap(currentLevel, enemy);
			break;
		}
		else {
			// Invalid menu input
			std::cout << "Invalid choice, try again.\n";
		}
	}
}


/****************************************************************
 *
 * FUNCTION resetMap
 *
 * Clears all current level data and reloads the level cleanly.
 *
 **************************************************************/
void resetMap(Level& currentLevel, Enemy& enemy)
{
	// Reset player and level progress
	playa.reset();
	currentLevel.resetCollectedTreasures();

	// Free dynamic map memory
	if (map != nullptr) {
		for (int i = 0; i < rows; ++i)
			delete[] map[i];
		delete[] map;
		map = nullptr;
	}

	// Delete all items
	for (auto& item : currentLevel.items)
		delete item;
	currentLevel.items.clear();

	// Clear enemies and walls
	enemy.enemies.clear();
	walls.clear();

	// Reset map dimensions
	rows = 0;
	cols = 0;

	// Reinitialize level
	startup_routines(currentLevel, enemy);
}


/****************************************************************
 *
 * FUNCTION render_screen
 *
 * Handles the main SFML rendering loop.
 * - Processes user input (keyboard & mouse)
 * - Updates game state (player, enemies, particles)
 * - Renders all visual elements
 * - Manages oxygen depletion and mission completion
 *
 **************************************************************/
void render_screen(Level& currentLevel, Enemy& enemy)
{
	unsigned int width = 800;
	unsigned int height = 800;

	sf::VideoMode mode(sf::Vector2u(width, height));
	sf::RenderWindow window(mode, "Holy Diver");
	window.setFramerateLimit(60);

	while (window.isOpen()) {
		std::optional<sf::Event> eventOpt;

		// Clock for frame delta time
		static sf::Clock deltaClock;

		// Clock for oxygen depletion (1 second ticks)
		static sf::Clock oxygenClock;

		float dt = deltaClock.restart().asSeconds();

		/********************
		 * EVENT PROCESSING
		 ********************/
		while ((eventOpt = window.pollEvent())) {
			sf::Event event = *eventOpt;

			// Window close event
			if (event.is<sf::Event::Closed>()) {
				window.close();
				openParticleSim = false;
				return; // return control to main()
			}

			// Mouse input: push particles away from cursor
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button(0))) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				sf::Vector2f mousePosF(
					static_cast<float>(mousePos.x),
					static_cast<float>(mousePos.y)
				);

				for (auto& p : particleSolver.getObjects()) {
					sf::Vector2f dir = p.position - mousePosF;
					float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
					if (length != 0) dir /= length;

					float strength = 20.0f;
					p.addVelocity(dir * strength, 1.0f / 60.0f);
				}
			}

			// Key pressed
			if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
				playa.ProcessEvents(keyEvent->code, true);
			}

			// Key released
			if (auto* keyEvent = event.getIf<sf::Event::KeyReleased>()) {
				playa.ProcessEvents(keyEvent->code, false);
			}
		}

		/********************
		 * OXYGEN HANDLING
		 ********************/
		if (oxygenClock.getElapsedTime().asSeconds() >= 1.0f) {
			playa.oxygenTime -= 1.0f;

			// Clamp oxygen to zero and apply damage
			if (playa.oxygenTime < 0.f) {
				playa.oxygenTime = 0.f;
				playa.takeDamage(2, window);
			}

			oxygenClock.restart();
		}

		// Update window title with player status
		window.setTitle(
			"Holy Diver - Oxygen & Health: " +
			std::to_string((int)playa.oxygenTime) + " s, " +
			std::to_string((int)playa.health) + " hp"
		);

		/********************
		 * UPDATE & RENDER
		 ********************/
		particleSolver.update(rectangle);

		window.clear(sf::Color(20, 20, 40));

		// Draw map border
		window.draw(rectangle);

		// Draw walls
		for (auto& wall : walls) {
			wall.draw(window);
		}

		// Draw items
		for (auto& item : currentLevel.items) {
			if (!item->collected)
				window.draw(item->shape);
		}

		// Update and draw enemies
		for (auto& e : enemy.enemies) {
			e.update(dt, playa.getPosition(), window, playa);
			e.draw(window);
		}

		// Draw particles
		for (Particle& p : particleSolver.getObjects())
			window.draw(p.shape);

		particleSolver.drawGrid(rectangle, window);

		// Update player physics and collisions
		playa.update(walls);

		/********************
		 * ITEM PICKUPS
		 ********************/
		for (auto& item : currentLevel.items) {
			if (!item->collected) {
				sf::Vector2f diff = item->position - playa.getPosition();
				float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
				float pickupRadius = 15.f;

				if (distance < pickupRadius) {
					item->applyEffect(playa);
					item->collected = true;
				}
			}
		}

		/********************
		 * LEVEL COMPLETION
		 ********************/
		int collectedCount = currentLevel.getCollectedTreasures();
		int totalCount = currentLevel.getTotalTreasures();

		if (collectedCount == currentLevel.items.size() &&
			!currentLevel.items.empty()) {

			bool lastLevel =
				(currentLevel.currentMapIndex ==
					currentLevel.mapFiles.size() - 1);

			if (lastLevel) {
				openParticleSim = false;
				window.close();

				std::string playerName;
				std::cout << "Congratulations! Thanks to you our company's shareholders' profits have tripled during the time of your diving. Enter your name for the company's Hall of Fame record: ";
				std::cin >> playerName;

				currentLevel.addCollectedToTotal(playa) += collectedCount;
				int totalTreasures = currentLevel.addCollectedToTotal(playa);
				int deathCount = playa.deaths;

				HallOfFame hof;
				hof.load();
				hof.addEntry({
					playerName,
					currentLevel.currentMapIndex + 1,
					totalTreasures,
					deathCount
					});
				hof.display();

				resetMap(currentLevel, enemy);
			}
			else {
				currentLevel.customMapFile = false;
				std::cout << "All minerals collected! Thanks to you our company's profits are rising! Ready for next mission?\n";
				currentLevel.addCollectedToTotal(playa) += collectedCount;
				currentLevel.currentMapIndex++;
				openParticleSim = false;
				window.close();
				resetMap(currentLevel, enemy);
			}
		}

		// Draw player last (on top)
		playa.drawTo(window);

		window.display();
	}
}
/****************************************************************
 *
 * FUNCTION start_splash_screen
 *
 * outputs any data or info at program start
 *
 * **************************************************************/

void start_splash_screen(Level& currentLevel)///// still need to add more instructions
{
	char input;
	/* this function to display any title information at startup, may include instructions or fancy ASCII-graphics */
	std::cout << "@@@  @@@   @@@@@@   @@@       @@@ @@@     @@@@@@@   @@@  @@@  @@@  @@@@@@@@  @@@@@@@   \n";
	std::cout << "@@@  @@@  @@@@@@@@  @@@       @@@ @@@     @@@@@@@@  @@@  @@@  @@@  @@@@@@@@  @@@@@@@@  \n";
	std::cout << "@!@  @@@  @@!  @@@  @@!       @@! !@@     @@!  @@@  @@!  @@!  @@@  @@!       @@!  @@@  \n";
	std::cout << "!@!  @!@  !@!  @!@  !@!       !@! @!!     !@!  @!@  !@!  !@!  @!@  !@!       !@!  @!@  \n";
	std::cout << "@!@!@!@!  @!@  !@!  @!!        !@!@!      @!@  !@!  !!@  @!@  !@!  @!!!:!    @!@!!@!   \n";
	std::cout << "!!!@!!!!  !@!  !!!  !!!         @!!!      !@!  !!!  !!!  !@!  !!!  !!!!!:    !!@!@!    \n";
	std::cout << "!!:  !!!  !!:  !!!  !!:         !!:       !!:  !!!  !!:  :!:  !!:  !!:       !!: :!!   \n";
	std::cout << ":!:  !:!  :!:  !:!   :!:        :!:       :!:  !:!  :!:   ::!!:!   :!:       :!:  !:!  \n";
	std::cout << "::   :::  ::::: ::   :: ::::     ::        :::: ::   ::    ::::     :: ::::  ::   :::  \n";
	std::cout << " :   : :   : :  :   : :: : :     :        :: :  :   :       :      : :: ::    :   : :  \n";

	std::cout << "\n";

	std::cout << std::endl << "WELCOME to epic Holy Diver v0.01 \n";
	std::cout << "Enter commands and enjoy! \n" << "\n";
	std::cout << "\n";
	std::cout << "Your Company, Profit Maxx, Needs You! \n";
	std::cout << "\n";
	std::cout << "We are sending you to three locations to explore deep ocean caves recently discovered by our research team.These caves contain valuable resources known as Hydra Minerals and Oxy Minerals. \n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "Please note : there may be side effects from prolonged exposure to these minerals.\n";
	std::cout << "\n";
	std::cout <<	"Your mission : Locate Hydra Minerals(yellow) and Oxy Minerals(white) while avoiding dangerous underwater creatures (red & pink). Do not underestimate the dangers of the ocean. They say some of the creatures down there can go through the cave walls. We are in the creatures' territories. And remember... sometimes the smartest move is not to move at all.\n";
	std::cout << "\n";
	std::cout << "\n";
	HallOfFame Hof;
	Hof.filename = "hall_of_fame.txt";
	Hof.load();
	Hof.display();
	std::cout << "Move with WASD, click with mouse to see in the depths and when you decide to quit remember to close the map window first and input q. You can see your oxygen level and health on the top right corner of the map window. You can add your own custom map, but if you load a custom map remember to quit and start the application over before going back to the original game levels. Loading your custom map is only possible in the beginning of the game. \n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "\n";

	
	std::cout << "Are you ready to hit the depths no human has ever yet seen?(y) Input (m) if you want to load your custom map: \n";
	std::cout << ">>>";  std::cin >> input;

	if (input == 'y' || input == 'Y') {
		openParticleSim = true;
		return;
	}
	if (input == 'm' || input == 'M')
	{
		currentLevel.customMapFile = true;
		std::cout << "Enter your map file name (with extension, e.g., mymap.txt): ";
		std::cin >> currentLevel.customMapFileName;
		return;
	}
}

/****************************************************************
 *
 * FUNCTION startup_routines
 *
 * Function performs any tasks necessary to set up a game
 * May contain game load dialogue, new player/game dialogue, level loading, random inits, whatever else
 *
 * At first week, this could be suitable place to load level map.
 *
 * **************************************************************/
void startup_routines(Level& currentLevel, Enemy& enemy) /// CHANGING MAP TO TO SFML SCREEN
{

	// Clear old particles
	particleSolver.getObjects().clear();
	
	int spawn_x = 0;
	int spawn_y = 0;
	// Load the map first
	
	load_level(currentLevel, enemy); // or ask user for filename

	unsigned int width = 840;
	unsigned int height = 840;

	// rectangle will be adjusted in load_level based on 'x' tiles
	rectangle.setFillColor(sf::Color::Black);
	rectangle.setOutlineThickness(5.0f);
	rectangle.setOutlineColor(sf::Color::Blue);
	
	playa.setFillColor(sf::Color::Green);

	
	// loop through the map to spawn particles and adjust rectangle
	
	float cellSize = 20.f; // how much space each charachter will create
	int particlesPerCell = 3; // how many particles per 'o' cell


	for (int i = 0; i < rows; i++) { // For loop for rows
		for (int j = 0; j < cols; j++) { // for loop for cols
			char cell = map[i][j]; // assigning char coordinate to charachter called cell to check what to create out of it
			float x = j * cellSize + cellSize / 2.f; // to get the exact corresponding coordinate in the screen we have to multiply it with the cellsize and to get to the center of that cell we have to add half of the size of cellsize
			float y = i * cellSize + cellSize / 2.f; // same as the earlier but for i

			if (cell == 'x') { // if the cell is oocupied by x we use rectangle to create the map boundaries
				rectangle.setSize(sf::Vector2f(cols * cellSize, rows * cellSize));
				rectangle.setOrigin(rectangle.getSize() / 2.f);
				rectangle.setPosition(sf::Vector2f(width / 2.f, height / 2.f));

				sf::Vector2f rectTopLeft = rectangle.getPosition() - rectangle.getSize() / 2.f;

				sf::Vector2f wallPos(
					rectTopLeft.x + j * cellSize,
					rectTopLeft.y + i * cellSize);

				walls.emplace_back(wallPos.x, wallPos.y, cellSize, cellSize);
			}
			else if (cell == 'B') { // Hydra Mineral
				// rectangle top-left position in window coordinates
				sf::Vector2f rectTopLeft = rectangle.getPosition() - rectangle.getSize() / 2.f;
				sf::Vector2f itemPos(
					rectTopLeft.x + j * cellSize + cellSize / 2.f,
					rectTopLeft.y + i * cellSize + cellSize / 2.f
				);
				currentLevel.items.push_back(new HydraMineral(itemPos));
			}
			else if (cell == 'O') { // Oxygen
				// rectangle top-left position in window coordinates
				sf::Vector2f rectTopLeft = rectangle.getPosition() - rectangle.getSize() / 2.f;
				sf::Vector2f itemPos(
					rectTopLeft.x + j * cellSize + cellSize / 2.f,
					rectTopLeft.y + i * cellSize + cellSize / 2.f
				);
				currentLevel.items.push_back(new Oxygen(itemPos));
			}
			else if (cell == 'o') { //if the cell is o we spawn int particlesPerCell amount of particles currently 4 randomly with in the boundaries
				for (int p = 0; p < particlesPerCell; p++) {
					// small random offset within the cell
					float offsetX = static_cast<float>(rand() % static_cast<int>(cellSize - 4)) - (cellSize / 2.f - 2.f);
					float offsetY = static_cast<float>(rand() % static_cast<int>(cellSize - 4)) - (cellSize / 2.f - 2.f);

					particleSolver.addObject(sf::Vector2f(x + offsetX, y + offsetY), 7.f);
				}
			}
			else if (cell=='P') {// if the cell is P create a square to represent the player
				spawn_x = i;
				spawn_y = j;
				playa.x = spawn_x;
				playa.y = spawn_y;

				// rectangle top-left position in window coordinates
				sf::Vector2f rectTopLeft = rectangle.getPosition() - rectangle.getSize() / 2.f;
				
				// Place player in the **center of the P-cell**
				float cellSize = 20.f;
				playa.setPosition(sf::Vector2f(
					rectTopLeft.x + spawn_y * cellSize + cellSize / 2.f,
					rectTopLeft.y + spawn_x * cellSize + cellSize / 2.f
				));
			}
			else if (cell == 'E') { // spawn regular moving enemy
				sf::Vector2f rectTopLeft = rectangle.getPosition() - rectangle.getSize() / 2.f;
				sf::Vector2f enemyPos(
					rectTopLeft.x + j * cellSize + cellSize / 2.f,
					rectTopLeft.y + i * cellSize + cellSize / 2.f
				);
				int enemyID = static_cast<int>(enemy.enemies.size()); // unique ID per enemy
				// spawn as normal moving enemy
				enemy.enemies.emplace_back(static_cast<int>(enemyPos.x), static_cast<int>(enemyPos.y), enemyID, Enemy::Type::Moving);
			}
			else if (cell == 'S') { // spawn oscillating enemy
				sf::Vector2f rectTopLeft = rectangle.getPosition() - rectangle.getSize() / 2.f;
				sf::Vector2f enemyPos(
					rectTopLeft.x + j * cellSize + cellSize / 2.f,
					rectTopLeft.y + i * cellSize + cellSize / 2.f
				);
				int enemyID = static_cast<int>(enemy.enemies.size()); // unique ID per enemy
				enemy.enemies.emplace_back(static_cast<int>(enemyPos.x), static_cast<int>(enemyPos.y), enemyID, Enemy::Type::Oscillating);
				// Change the color to pink
				enemy.enemies.back().shape.setFillColor(sf::Color(255, 105, 180)); // RGB pink
			}
			
		}
	}
	
}


/****************************************************************
 *
 * FUNCTION quit_routines
 *
 * function performs any routines necessary at program shut-down, such as freeing memory or storing data files
 *
 * **************************************************************/
void quit_routines(Level& currentLevel)
{

	// (*) ... the memory should be free'ed here at latest.

	// Free dynamically allocated map memory
		if (map != nullptr) {
			for (int i = 0; i < rows; ++i) {
				delete[] map[i];   // delete each row
			}
			delete[] map;          // delete the array of pointers
			map = nullptr;         // good practice to avoid dangling pointer
		}

		// Free dynamically allocated Item objects
		for (auto& item : currentLevel.items) {
			delete item;
		}
		currentLevel.items.clear();

		// Clear walls vector (automatic cleanup)
		walls.clear();

	std::cout << "\n" << "BYE! Welcome back soon." << "\n";
}




