#include "Scene_Menu.h"
#include "Scene_Game.h"
#include "MusicPlayer.h"
#include <memory>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

void Scene_Menu::onEnd()
{
	_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}

std::vector<std::string> subtitles = {
	"Tariffs: 300% on MAGA Caps!",
	"Tariffs... Pay now or get deported.",
	"Make Games Great Again!",
	"This game is protected by 25% tariff!\n Pay up or turn back!",
	"Canada says no to tariffs, eh? Try again later!",
	"Gulf of America!",
	"Make Tariffs Great Again!",
	"Tariffs delayed... again",
	"Elbows Up!",
	"Tariffs for Penguins Too!"
};

std::string randomSubtitle;

void Scene_Menu::centerTexture(sf::Sprite& sprite)
{
	sf::FloatRect bounds = sprite.getLocalBounds();
	sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
}

void Scene_Menu::centerText(sf::Text& text)
{
	sf::FloatRect textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f);
}

void Scene_Menu::init()
{
	// initializing random numbers
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	int randomIndex = std::rand() % subtitles.size();
	randomSubtitle = subtitles[randomIndex];

	MusicPlayer::getInstance().play("menuTheme");
	MusicPlayer::getInstance().setVolume(5);

	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
	registerAction(sf::Keyboard::Space, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");

	m_title = "Dennis Trumpet";

	m_menuStrings.push_back("Level 1");
	m_menuStrings.push_back("Level 2");
	m_menuStrings.push_back("Instructions");

	m_levelPaths.push_back("../level.txt");
	m_levelPaths.push_back("../level2.txt");
	m_levelPaths.push_back("../instructions.txt");

	m_menuText.setFont(Assets::getInstance().getFont("PressStart"));

	const size_t CHAR_SIZE{ 84 };
	m_menuText.setCharacterSize(CHAR_SIZE);

	if (!m_Texture.loadFromFile("../assets/textures/gooseVeagle.png"))
	{
		std::cout << "Error loading texture\n";
	}
	m_Sprite.setTexture(m_Texture);
	centerTexture(m_Sprite);

}

void Scene_Menu::update(sf::Time dt)
{
	_entityManager.update();


	if (m_blinkClock.getElapsedTime().asSeconds() >= 0.6f)
	{
		m_isSubtitleVisible = !m_isSubtitleVisible;

		m_blinkClock.restart();
	}
}


void Scene_Menu::sRender()
{
	static const sf::Color backgroundColor(23, 167, 168);

	_game->window().clear(sf::Color(backgroundColor));

	sf::View view = _game->window().getView();
	view.setCenter(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f);
	_game->window().setView(view);

	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color normalColor(0, 0, 0);

	//static const sf::Color backgroundColor(100, 100, 255);


	if (m_showConstructionMessage)
	{
		sf::RectangleShape overlay(sf::Vector2f(_game->window().getSize()));
		overlay.setFillColor(sf::Color(0, 0, 0, 200));
		_game->window().draw(overlay);

		m_Sprite.setPosition(_game->window().getSize().x / 2.f,
			_game->window().getSize().y / 2.f - 100);
		_game->window().draw(m_Sprite);

		sf::Text messageText;
		messageText.setFont(Assets::getInstance().getFont("PressStart"));
		messageText.setCharacterSize(50);
		messageText.setFillColor(sf::Color::Yellow);
		messageText.setString("The 51st State coming soon...");
		centerText(messageText);
		messageText.setPosition(_game->window().getSize().x / 2.f,
			_game->window().getSize().y / 2.f + 150);
		_game->window().draw(messageText);

		sf::Text continueText;
		continueText.setFont(Assets::getInstance().getFont("PressStart"));
		continueText.setCharacterSize(30);
		continueText.setFillColor(sf::Color::White);
		continueText.setString("PRESS ANY KEY TO MAKE GAMES GREAT AGAIN");
		centerText(continueText);
		continueText.setPosition(_game->window().getSize().x / 2.f,
			_game->window().getSize().y - 100);
		_game->window().draw(continueText);

		return;
	}

	if (m_showInstructions)
	{
		sf::RectangleShape overlay(sf::Vector2f(_game->window().getSize()));
		overlay.setFillColor(sf::Color(0, 0, 0, 200));
		_game->window().draw(overlay);

		sf::Text title("CONTROLS", Assets::getInstance().getFont("PressStart"), 80);
		title.setFillColor(sf::Color::White);
		centerText(title);
		title.setPosition(_game->window().getSize().x / 2.f, 150);
		_game->window().draw(title);

		std::vector<std::string> controls = {
			"Collect enough fanatics to seize power!",
			"W / Up Arrow: Jump",
			"A / Left Arrow: Move Left",
			"D / Right Arrow: Move Right",
			"Space: Shoot",
			"P: Pause Game",
			"ESC: Exit Game"
		};

		sf::Text controlText;
		controlText.setFont(Assets::getInstance().getFont("PressStart"));
		controlText.setCharacterSize(40);
		controlText.setFillColor(sf::Color::White);

		float startY = 250;
		float spacing = 60;

		for (size_t i = 0; i < controls.size(); ++i)
		{
			controlText.setString(controls[i]);
			centerText(controlText);
			controlText.setPosition(_game->window().getSize().x / 2.f, startY + (i * spacing));
			_game->window().draw(controlText);
		}


		sf::Text continueText("Press any key to return to menu",
			Assets::getInstance().getFont("PressStart"), 30);
		continueText.setFillColor(sf::Color::White);
		centerText(continueText);
		continueText.setPosition(_game->window().getSize().x / 2.f,
			_game->window().getSize().y - 100);
		_game->window().draw(continueText);

		return;
	}


	sf::Text footer("UP: W    DOWN: S   PLAY: SPACE    QUIT: ESC",
		Assets::getInstance().getFont("megaman"), 40);
	footer.setFillColor(normalColor);
	footer.setPosition(32, 975);


	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_title);
	m_menuText.setPosition(10, 25);
	_game->window().draw(m_menuText);

	if (m_isSubtitleVisible) {
		sf::Text subtitle;
		subtitle.setFont(Assets::getInstance().getFont("PressStart"));
		subtitle.setCharacterSize(40);
		subtitle.setFillColor(normalColor);
		subtitle.setString(randomSubtitle);
		subtitle.setPosition(10, 155);
		_game->window().draw(subtitle);
	}


	for (size_t i{ 0 }; i < m_menuStrings.size(); ++i)
	{
		m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
		m_menuText.setPosition(32, 180 + (i + 1) * 150);
		m_menuText.setString(m_menuStrings.at(i));
		_game->window().draw(m_menuText);
	}

	_game->window().draw(footer);

}


void Scene_Menu::sDoAction(const Command& action)
{
	if (m_showInstructions)
	{
		if (action.type() == "START")
		{
			// Cualquier tecla nos lleva de vuelta al menú
			m_showInstructions = false;
			return;
		}
	}
	else if (m_showConstructionMessage)
	{
		if (action.type() == "START") {
			m_showConstructionMessage = false;
			return;
		}

	}
	else if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
		}
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			// level 2
			if (m_menuIndex == 1) {
				m_showConstructionMessage = true;
			}
			else if (m_menuIndex == 2) { // instructions
				m_showInstructions = true;
			}
			else {
				std::cout << "Opening Scene_Game...\n";

				_game->changeScene("PLAY", std::make_shared<Scene_Game>(_game, m_levelPaths[m_menuIndex]));

				std::cout << "Scene_game opened succesfully.\n";
			}

		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}