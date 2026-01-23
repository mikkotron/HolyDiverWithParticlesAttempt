/*
*
* Holy diver - an epic adventure at object-oriented world way beneath the surface!
* Template code for implementing the rich features to be specified later on.
*
*/

// NOTE: This project uses SFML (Graphics, Window) and requires linking against the SFML libraries.
// Tested with SFML3.0.0 and C++17


#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <cmath>
#include <fstream>
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
#include "LevelBuilder.hpp"
#include "HallOfFame.hpp"
#include "GameData.hpp"




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



// ------------------- Function declarations -------------------
InputAction read_input(char* input, Level& currentLevel, GameData& gameData);
void handleAbortMission(Level& currentLevel, Enemy& enemy, GameData& gameData);
void render_screen( Level& currentLevel, Enemy& enemy, GameData& gameData);
void start_splash_screen(Level& currentLevel, GameData& gameData);
void quit_routines(Level& currentLevel, GameData& gameData);


// ------------------- Main -------------------
char input;

int main(void)
{
	GameData gameData; // contains player, particleSolver, walls, isRendering
	Level currentLevel;
	Enemy enemy({ 200.f, 200.f }, 0);

	// Show splash screen and instructions
	start_splash_screen(currentLevel, gameData);


	// Load the level (reads map and spawns objects)
	currentLevel.load(gameData, enemy); // Only pass the full GameData object





	// IMPORTANT NOTE: do not exit program without cleanup
	while (true) {
		// Read player input (console-based)
		InputAction action = read_input(&input, currentLevel, gameData);

		// Handle input actions
		switch (action) {
		case InputAction::QuitGame:
			quit_routines(currentLevel, gameData);
			return 0;

		case InputAction::RestartLevel:
			currentLevel.load(gameData, enemy);

			break;

		case InputAction::AbortMission:
			handleAbortMission(currentLevel, enemy, gameData);
			break;

		case InputAction::None:
			break;
		}

		// Launch SFML rendering loop when allowed
		if (gameData.isRendering)
			render_screen(currentLevel, enemy, gameData);
	}

	// Safety cleanup (normally unreachable)
	quit_routines(currentLevel, gameData);
	return 0;
}


/****************************************************************
 *
 * FUNCTION read_input
 *
 * Reads console input from the user and converts it into a game action.
 * If the player is dead, the mission is automatically aborted.
 *
 **************************************************************/
InputAction read_input(char* input, Level& currentLevel, GameData& gameData)
{
	// If player is alive, show available commands
	if (!gameData.player.isDead())
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
		gameData.isRendering = true;   // allow rendering loop to start
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
void handleAbortMission(Level& currentLevel, Enemy& enemy, GameData& gameData)
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
		!gameData.player.isDead();

	if (!gameData.player.isDead()) {
		// Add partial progress to total score
		currentLevel.addCollectedToTotal(gameData.player) += endResult;

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
		gameData.player.reset();
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
			currentLevel.load(gameData, enemy);
			break;
		}
		else if (choice == 'p' && currentLevel.currentMapIndex > 0) {
			// Go back to previous level
			currentLevel.customMapFile = false;
			currentLevel.currentMapIndex--;
			currentLevel.load(gameData, enemy);
			break;
		}
		else if (choice == 'n' && canAdvance) {
			// Advance to next level
			currentLevel.customMapFile = false;
			currentLevel.currentMapIndex++;
			currentLevel.load(gameData, enemy);
			break;
		}
		else if (choice == 'q') {
			// Quit to menu (stop rendering loop)
			gameData.isRendering = false;
			currentLevel.load(gameData, enemy);
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
 * FUNCTION render_screen
 *
 * Handles the main SFML rendering loop.
 * - Processes user input (keyboard & mouse)
 * - Updates game state (player, enemies, particles)
 * - Renders all visual elements
 * - Manages oxygen depletion and mission completion
 *
 **************************************************************/
void render_screen(Level& currentLevel, Enemy& enemy, GameData& gameData)
{
	


	unsigned int width = 800;
	unsigned int height = 800;

	sf::VideoMode mode(sf::Vector2u(width, height));
	sf::RenderWindow window(mode, "Holy Diver");
	window.setFramerateLimit(60);

	// Set death callback once
	gameData.player.onDeath = [&]() {
		std::cout << "PLAYER DIED!\n";
		gameData.player.deaths++;
		gameData.isRendering = false;
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
				gameData.isRendering = false;
				return; // return control to main()
			}

			// Mouse input: push particles away from cursor
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button(0))) {
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				sf::Vector2f mousePosF(
					static_cast<float>(mousePos.x),
					static_cast<float>(mousePos.y)
				);

				for (auto& p : gameData.particleSolver.getObjects()) {
					sf::Vector2f dir = p.position - mousePosF;
					float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
					if (length != 0) dir /= length;

					float strength = 20.0f;
					p.addVelocity(dir * strength, 1.0f / 60.0f);
				}
			}

			// Key pressed
			if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
				gameData.player.handleInput(keyEvent->code, true);
			}

			// Key released
			if (auto* keyEvent = event.getIf<sf::Event::KeyReleased>()) {
				gameData.player.handleInput(keyEvent->code, false);
			}
		}

		/********************
		 * OXYGEN HANDLING
		 ********************/
		if (oxygenClock.getElapsedTime().asSeconds() >= 1.0f) {
			gameData.player.oxygenTime -= 1.0f;

			// Clamp oxygen to zero and apply damage
			if (gameData.player.oxygenTime < 0.f) {
				gameData.player.oxygenTime = 0.f;
				gameData.player.takeDamage(2);
			}

			oxygenClock.restart();
		}

		// Update window title with player status
		window.setTitle(
			"Holy Diver - Oxygen & Health: " +
			std::to_string((int)gameData.player.oxygenTime) + " s, " +
			std::to_string((int)gameData.player.health) + " hp"
		);

		/********************
		 * UPDATE & RENDER
		 ********************/
		gameData.particleSolver.update(currentLevel.bounds);


		window.clear(sf::Color(20, 20, 40));

		// Draw map border
		window.draw(currentLevel.bounds);

		// Draw walls
		for (auto& wall : gameData.walls) {
			wall.draw(window);
		}

		// Draw items
		for (auto& item : currentLevel.items) {
			if (!item->collected)
				window.draw(item->shape);
		}

		// Update and draw enemies
		for (auto& e : currentLevel.enemies) {
			e.update(gameData.walls, currentLevel.bounds);  // movement + collisions
			e.updateAI(dt, gameData.player);          // chasing, oscillation, damage
			e.draw(window);                 // render
		}


		// Draw particles
		for (Particle& p : gameData.particleSolver.getObjects())
			window.draw(p.shape);

		gameData.particleSolver.drawGrid(currentLevel.bounds, window);

		// Update player physics and collisions
		gameData.player.update(gameData.walls, currentLevel.bounds);

		/********************
		 * ITEM PICKUPS
		 ********************/
		for (auto& item : currentLevel.items) {
			if (!item->collected) {
				sf::Vector2f diff = item->position - gameData.player.getPosition();
				float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
				float pickupRadius = 15.f;

				if (distance < pickupRadius) {
					item->applyEffect(gameData.player);
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
				gameData.isRendering = false;
				window.close();

				std::string playerName;
				std::cout << "Congratulations! Thanks to you our company's shareholders' profits have tripled during the time of your diving. Enter your name for the company's Hall of Fame record: ";
				std::cin >> playerName;

				currentLevel.addCollectedToTotal(gameData.player) += collectedCount;
				int totalTreasures = currentLevel.addCollectedToTotal(gameData.player);
				int deathCount = gameData.player.deaths;

				HallOfFame hof;
				hof.load();
				hof.addEntry({
					playerName,
					currentLevel.currentMapIndex + 1,
					totalTreasures,
					deathCount
					});
				hof.display();

				currentLevel.load(gameData, enemy);
			}
			else {
				currentLevel.customMapFile = false;
				std::cout << "All minerals collected! Thanks to you our company's profits are rising! Ready for next mission?\n";
				currentLevel.addCollectedToTotal(gameData.player) += collectedCount;
				currentLevel.currentMapIndex++;
				gameData.isRendering = false;
				window.close();
				currentLevel.load(gameData, enemy);
			}
		}

		// Draw player last (on top)
		gameData.player.draw(window);

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

void start_splash_screen(Level& currentLevel, GameData& gameData)///// still need to add more instructions
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
	HallOfFame Hof ("hall_of_fame.txt");
	Hof.load();
	Hof.display();
	std::cout << "Move with WASD, click with mouse to see in the depths and when you decide to quit remember to close the map window first and input q. You can see your oxygen level and health on the top right corner of the map window. You can add your own custom map, but if you load a custom map remember to quit and start the application over before going back to the original game levels. Loading your custom map is only possible in the beginning of the game. \n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "\n";

	
	std::cout << "Are you ready to hit the depths no human has ever yet seen?(y) Input (m) if you want to load your custom map: \n";
	std::cout << ">>>";  std::cin >> input;

	if (input == 'y' || input == 'Y') {
		gameData.isRendering = true;
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
 * FUNCTION quit_routines
 *
 * function performs any routines necessary at program shut-down, such as freeing memory or storing data files
 *
 * **************************************************************/
void quit_routines(Level& currentLevel, GameData& gameData)
{
	// Free dynamically allocated map memory
	currentLevel.freeMap();

	// Clear items
	currentLevel.items.clear();

	// Clear enemies
	currentLevel.enemies.clear();

	// Clear walls vector (global)
	gameData.walls.clear();

	// Clear particle solver objects
	gameData.particleSolver.getObjects().clear();

	// Reset map indices
	currentLevel.currentMapIndex = 0;

	// Reset player (optional)
	gameData.player.reset();

	std::cout << "\nBYE! Welcome back soon.\n";
}




