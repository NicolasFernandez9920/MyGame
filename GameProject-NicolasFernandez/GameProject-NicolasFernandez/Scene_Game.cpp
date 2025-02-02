#include "Scene_Game.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
//#include "MusicPlayer.h"
#include "Assets.h"
//#include "SoundPlayer.h"
#include "GameEngine.h"


#include <random>
#include <fstream>
#include <iostream>

Scene_Game::Scene_Game(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, _worldView(gameEngine->window().getDefaultView())
{
	std::cout << "Scene_Game constructor initialized\n";

	init(levelPath);

	std::cout << "Scene_Game constructor done.\n";
}

void Scene_Game::init(const std::string& levelPath)
{

	loadLevel(levelPath);


	sf::Vector2f view{ _worldView.getSize().x / 2.f, _worldBounds.height - _worldView.getSize().y / 2.f };

	_worldView.setCenter(view);

	spawnPlayer(_worldView.getCenter());

}

void Scene_Game::sUpdate(sf::Time dt)
{
	_entityManager.update();
}

void Scene_Game::onEnd()
{
}

void Scene_Game::spawnPlayer(sf::Vector2f pos)
{
	_player = _entityManager.addEntity("player");
	_player->addComponent<CTransform>(pos);

	auto& sr = Assets::getInstance().getSpriteRec("playerDT");
	auto& sprite = _player->addComponent<CSprite>(Assets::getInstance().getTexture(sr.texName)).sprite;
	sprite.setTextureRect(sr.texRect);
	centerOrigin(sprite);

	_player->addComponent<CBoundingBox>(sf::Vector2f{ 30.f, 30.f });
	_player->addComponent<CState>("straight");
	_player->addComponent<CInput>();
	_player->addComponent<CPlayerState>();
}



void Scene_Game::loadLevel(const std::string& path)
{
	std::ifstream config(path);

	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	std::string token{ "" };
	config >> token;
	while (!config.eof()) {
		if (token == "Bkg") {
			std::string name;
			sf::Vector2f pos;
			config >> name >> pos.x >> pos.y;

			std::cout << "Loading background: " << name << " at position (" << pos.x << ", " << pos.y << ")\n";

			auto e = _entityManager.addEntity("bkg");

			// for background, no textureRect its just the whole texture
			// and no center origin, position by top left corner
			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			if (!sprite.getTexture()) {
				std::cout << "Texture: " << name << " failed loading\n";
			}
			else {
				std::cout << "Texture: " << name << " loaded correctly\n";
			}
			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(pos);
		}
		else if (token == "World") {
			config >> _worldBounds.width >> _worldBounds.height;
		}
		//else if (token == "PlayerSpeed") {
		//	config >> _config.playerSpeed;
		//}
		config >> token;
	}

	config.close();
}


void Scene_Game::update(sf::Time dt)
{
	sUpdate(dt);
}

void Scene_Game::sDoAction(const Command& command)
{
}

void Scene_Game::sRender()
{
	_game->window().setView(_worldView);

	// draw bkg first
	for (auto e : _entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			auto& sprite = e->getComponent<CSprite>().sprite;
			_game->window().draw(sprite);
		}
	}

	for (auto& e : _entityManager.getEntities()) {
		if (!e->hasComponent<CSprite>() || e->getTag() == "bkg")
			continue;

		// Draw Sprite
		auto& sprite = e->getComponent<CSprite>().sprite;
		auto& tfm = e->getComponent<CTransform>();
		sprite.setPosition(tfm.pos);
		sprite.setRotation(tfm.angle);
		_game->window().draw(sprite);

	}
}
