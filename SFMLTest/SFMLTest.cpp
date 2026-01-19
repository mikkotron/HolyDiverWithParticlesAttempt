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
#include "Player.hpp"
#include "Wall.hpp"
#include "Entity.hpp"
#include "MathUtils.hpp"
#include "Enemy.hpp"
#include "Particle.hpp"
#include "Solver.hpp"
#include "Item.hpp"
#include "Items.hpp"
#include <memory>
#include "Level.hpp"



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



// ------------------- Globals -------------------
// Toggle for particle simulation mode
bool openParticleSim = false;

// Particle physics solver
Solver particleSolver;

// Map dimensions
int rows = 0;
int cols = 0;

// Tile map data
char** map = nullptr;

// Collection of walls for cave collision detection for the player
std::vector<class Wall> walls;

// Global player instance
Player playa(20.f, 20.f);

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
	Enemy enemy({ 200.f, 200.f }, 0);

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

	
	currentLevel.items.clear();

	// Clear enemies and walls
	currentLevel.enemies.clear();
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


	currentLevel.items.clear();

	// Clear enemies and walls
	currentLevel.enemies.clear();
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

	// Set death callback once
	playa.onDeath = [&]() {
		std::cout << "PLAYER DIED!\n";
		playa.deaths++;
		openParticleSim = false;
		window.close();
		};

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
				playa.handleInput(keyEvent->code, true);
			}

			// Key released
			if (auto* keyEvent = event.getIf<sf::Event::KeyReleased>()) {
				playa.handleInput(keyEvent->code, false);
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
				playa.takeDamage(2);
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
		particleSolver.update(currentLevel.bounds);

		window.clear(sf::Color(20, 20, 40));

		// Draw map border
		window.draw(currentLevel.bounds);

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
		for (auto& e : currentLevel.enemies) {
			e.update(walls, currentLevel.bounds);  // movement + collisions
			e.updateAI(dt, playa);          // chasing, oscillation, damage
			e.draw(window);                 // render
		}


		// Draw particles
		for (Particle& p : particleSolver.getObjects())
			window.draw(p.shape);

		particleSolver.drawGrid(currentLevel.bounds, window);

		// Update player physics and collisions
		playa.update(walls, currentLevel.bounds);

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
		playa.draw(window);

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
	currentLevel.bounds.setFillColor(sf::Color::Black);
	currentLevel.bounds.setOutlineThickness(5.0f);
	currentLevel.bounds.setOutlineColor(sf::Color::Blue);
	
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
				// Set world bounds for clamping
				currentLevel.bounds.setSize(sf::Vector2f(cols * cellSize, rows * cellSize));
				currentLevel.bounds.setOrigin(currentLevel.bounds.getSize() / 2.f);
				currentLevel.bounds.setPosition(sf::Vector2f(width / 2.f, height / 2.f));

				sf::Vector2f rectTopLeft = currentLevel.bounds.getPosition() - currentLevel.bounds.getSize() / 2.f;

				sf::Vector2f wallPos(
					rectTopLeft.x + j * cellSize + cellSize / 2.f,
					rectTopLeft.y + i * cellSize + cellSize / 2.f
				);
				walls.emplace_back(wallPos.x, wallPos.y, cellSize, cellSize);

			}
			else if (cell == 'B') { // Hydra Mineral
				// rectangle top-left position in window coordinates
				sf::Vector2f rectTopLeft = currentLevel.bounds.getPosition() - currentLevel.bounds.getSize() / 2.f;
				sf::Vector2f itemPos(
					rectTopLeft.x + j * cellSize + cellSize / 2.f,
					rectTopLeft.y + i * cellSize + cellSize / 2.f
				);
				currentLevel.items.push_back(std::make_unique<HydraMineral>(itemPos));
			}
			else if (cell == 'O') { // Oxygen
				// rectangle top-left position in window coordinates
				sf::Vector2f rectTopLeft = currentLevel.bounds.getPosition() - currentLevel.bounds.getSize() / 2.f;

				sf::Vector2f itemPos(
					rectTopLeft.x + j * cellSize + cellSize / 2.f,
					rectTopLeft.y + i * cellSize + cellSize / 2.f
				);

				currentLevel.items.push_back(std::make_unique<Oxygen>(itemPos));
			}

			else if (cell == 'o') { //if the cell is o we spawn int particlesPerCell amount of particles currently 4 randomly with in the boundaries
				for (int p = 0; p < particlesPerCell; p++) {
					// small random offset within the cell
					float offsetX = static_cast<float>(rand() % static_cast<int>(cellSize - 4)) - (cellSize / 2.f - 2.f);
					float offsetY = static_cast<float>(rand() % static_cast<int>(cellSize - 4)) - (cellSize / 2.f - 2.f);

					particleSolver.addObject(sf::Vector2f(x + offsetX, y + offsetY), 7.f);
				}
			}
			else if (cell == 'P') { // Player spawn
				spawn_x = i;
				spawn_y = j;

				// rectangle top-left position in window coordinates
				sf::Vector2f rectTopLeft =
					currentLevel.bounds.getPosition() - currentLevel.bounds.getSize() / 2.f;

				// Place player in the center of the P-cell
				float cellSize = 20.f;
				playa.setPosition(sf::Vector2f(
					rectTopLeft.x + spawn_y * cellSize + cellSize / 2.f,
					rectTopLeft.y + spawn_x * cellSize + cellSize / 2.f
				));
			}

			else if (cell == 'E') {
				sf::Vector2f enemyPos(
					currentLevel.bounds.getPosition().x - currentLevel.bounds.getSize().x / 2.f + j * cellSize + cellSize / 2.f,
					currentLevel.bounds.getPosition().y - currentLevel.bounds.getSize().y / 2.f + i * cellSize + cellSize / 2.f
				);
				int enemyID = static_cast<int>(currentLevel.enemies.size());
				currentLevel.enemies.emplace_back(enemyPos, enemyID, Enemy::Type::Moving);
			}

			else if (cell == 'S') {
				sf::Vector2f enemyPos(
					currentLevel.bounds.getPosition().x - currentLevel.bounds.getSize().x / 2.f + j * cellSize + cellSize / 2.f,
					currentLevel.bounds.getPosition().y - currentLevel.bounds.getSize().y / 2.f + i * cellSize + cellSize / 2.f
				);
				int enemyID = static_cast<int>(currentLevel.enemies.size());
				currentLevel.enemies.emplace_back(enemyPos, enemyID, Enemy::Type::Oscillating);
				currentLevel.enemies.back().setColor(sf::Color(255, 105, 180)); // pink
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

		
		currentLevel.items.clear();

		// Clear walls vector (automatic cleanup)
		walls.clear();

	std::cout << "\n" << "BYE! Welcome back soon." << "\n";
}




