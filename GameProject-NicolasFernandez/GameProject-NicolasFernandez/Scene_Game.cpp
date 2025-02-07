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

	init(levelPath);

}

void Scene_Game::init(const std::string& levelPath)
{
	registerActions();

	loadLevel(levelPath);


	sf::Vector2f view{ _worldView.getSize().x / 2.f, _worldBounds.height - _worldView.getSize().y / 2.f };

	_worldView.setCenter(view);

	spawnPlayer(_worldView.getCenter());

}

void Scene_Game::sUpdate(sf::Time dt)
{
	_entityManager.update();
}

void Scene_Game::sMovement()
{
	// player movement
	auto& pt = _player->getComponent<CTransform>();
	pt.vel.x = 0.f;

	if (_player->getComponent<CInput>().left)
		pt.vel.x -= 1;

	if (_player->getComponent<CInput>().right)
		pt.vel.x += 1;

	if (_player->getComponent<CInput>().up) {
		_player->getComponent<CInput>().up = false;
		pt.vel.y = -_playerConfig.JUMP;
	}

	// facing direction
	if (pt.vel.x < -0.1)
		_player->getComponent<CState>().set(CState::isFacingLeft);
	if (pt.vel.x > 0.1)
		_player->getComponent<CState>().unSet(CState::isFacingLeft);


	// move all entities
	for (auto e : _entityManager.getEntities()) {
		auto& tx = e->getComponent<CTransform>();
		tx.prevPos = tx.pos;
		tx.pos += tx.vel;
	}
}

void Scene_Game::onEnd()
{
}

void Scene_Game::spawnTweet(sf::Vector2f dir)
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

void Scene_Game::playerMovement()
{
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

void Scene_Game::registerActions()
{
	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "BACK");
	registerAction(sf::Keyboard::Q, "QUIT");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");

	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::W, "JUMP");
	registerAction(sf::Keyboard::Up, "JUMP");
	registerAction(sf::Keyboard::Space, "SHOOT");
}


void Scene_Game::update(sf::Time dt)
{
	sUpdate(dt);
}

void Scene_Game::sDoAction(const Command& command)
{
	// On Key Press
	if (command.type() == "START") {
		if (command.name() == "PAUSE") { setPaused(!_isPaused); }
		else if (command.name() == "QUIT") { _game->quitLevel(); }

		else if (command.name() == "TOGGLE_TEXTURE") { _drawTextures = !_drawTextures; }
		else if (command.name() == "TOGGLE_COLLISION") { _drawAABB = !_drawAABB; }
		else if (command.name() == "TOGGLE_GRID") { _drawGrid = !_drawGrid; }

		// Player control
		else if (command.name() == "LEFT") { _player->getComponent<CInput>().left = true; }
		else if (command.name() == "RIGHT") { _player->getComponent<CInput>().right = true; }

		else if (command.name() == "JUMP") {
			if (_player->getComponent<CInput>().canJump) {
				_player->getComponent<CInput>().up = true;
				_player->getComponent<CInput>().canJump = false;
			}
		}
		else if (command.name() == "SHOOT") {
			if (_player->getComponent<CInput>().canShoot) {
				_player->getComponent<CInput>().shoot = true;
				_player->getComponent<CInput>().canShoot = false;
			}
		}
	}

	// on Key Release
	else if (command.type() == "END") {
		// Player control
		if (command.name() == "LEFT") { _player->getComponent<CInput>().left = false; }
		else if (command.name() == "RIGHT") { _player->getComponent<CInput>().right = false; }
		else if (command.name() == "JUMP") { _player->getComponent<CInput>().up = false; }
		else if (command.name() == "SHOOT") { _player->getComponent<CInput>().canShoot = true; }
	}
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
