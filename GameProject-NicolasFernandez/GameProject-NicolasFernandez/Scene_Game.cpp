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

	spawnPlayer();

}

void Scene_Game::sUpdate(sf::Time dt)
{
	_entityManager.update();

	sMovement();
	sCollision();
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

	// gravity
	pt.vel.y += _playerConfig.GRAVITY;
	pt.vel.x = pt.vel.x * _playerConfig.SPEED;

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

void Scene_Game::sCollision()
{
	// player with tile
	auto players = _entityManager.getEntities("player");
	auto tiles = _entityManager.getEntities("tile");

	for (auto p : players) {
		p->getComponent<CState>().unSet(CState::isGrounded); // not grounded
		for (auto t : tiles) {
			auto overlap = Physics::getOverlap(p, t);
			if (overlap.x > 0 && overlap.y > 0) // +ve overlap in both x and y means collision
			{
				auto prevOverlap = Physics::getPreviousOverlap(p, t);
				auto& ptx = p->getComponent<CTransform>();
				auto ttx = t->getComponent<CTransform>();


				// collision is in the y direction
				if (prevOverlap.x > 0) {
					if (ptx.prevPos.y < ttx.prevPos.y) {
						// player standing on something isGrounded
						p->getComponent<CTransform>().pos.y -= overlap.y;
						p->getComponent<CInput>().canJump = true;
						p->getComponent<CState>().set(CState::isGrounded);
					}
					else {
						// player hit something from below
						p->getComponent<CTransform>().pos.y += overlap.y;
					}
					p->getComponent<CTransform>().vel.y = 0.f;
				}


				// collision is in the x direction
				if (prevOverlap.y > 0) {
					if (ptx.prevPos.x < ttx.prevPos.x) // player left of tile
						p->getComponent<CTransform>().pos.x -= overlap.x;
					else
						p->getComponent<CTransform>().pos.x += overlap.x;
				}
			}
		}
	}
}

void Scene_Game::onEnd()
{
}

void Scene_Game::spawnTweet(sf::Vector2f dir)
{
}

void Scene_Game::spawnPlayer()
{
	_player = _entityManager.addEntity("player");
	_player->addComponent<CTransform>(gridToMidPixel(_playerConfig.X, _playerConfig.Y, _player));

	auto& sr = Assets::getInstance().getSpriteRec("playerDT");
	auto& sprite = _player->addComponent<CSprite>(Assets::getInstance().getTexture(sr.texName)).sprite;
	sprite.setTextureRect(sr.texRect);
	centerOrigin(sprite);

	_player->addComponent<CBoundingBox>(sf::Vector2f(_playerConfig.CW, _playerConfig.CH));
	_player->addComponent<CState>();
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
		else if (token == "Tile") {
			std::string name;
			float gx, gy;
			config >> name >> gx >> gy;

			auto e = _entityManager.addEntity("tile");
			e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			e->addComponent<CBoundingBox>(Assets::getInstance().getTexture(name).getSize());
			auto pos = gridToMidPixel(gx, gy, e);
			std::cout << "Tile at (" << pos.x << ", " << pos.y << ")\n";
			e->addComponent<CTransform>(pos);


		}
		else if (token == "Player") {
			config >>
				_playerConfig.X >>
				_playerConfig.Y >>
				_playerConfig.CW >>
				_playerConfig.CH >>
				_playerConfig.SPEED >>
				_playerConfig.JUMP >>
				_playerConfig.MAXSPEED >>
				_playerConfig.GRAVITY;
		}
		else if (token == "World") {
			config >> _worldBounds.width >> _worldBounds.height;
		}

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

sf::Vector2f Scene_Game::gridToMidPixel(float gridX, float gridY, sPtrEntt entity)
{
	// (left, bot) of grix,gidy)

	// this is for side scroll, and based on window height being the same as world height
	// to be more generic and support scrolling up and down as well as left and right it
	// should be based on world size not window size
	float x = 0.f + gridX * _gridCellSize.x;
	float y = 768.f - gridY * _gridCellSize.y;
	//float y = (_game->window().getSize().y / 2.f) - gridY * _gridCellSize.y;

	sf::Vector2f spriteSize = entity->getComponent<CAnimation>().animation.getSize();
	return sf::Vector2f(x + spriteSize.x / 2.f, y - spriteSize.y / 2.f);
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
			if (_player->getComponent<CInput>().canJump && _player->getComponent<CState>().test(CState::isGrounded)) {
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
	// set the view to center on the player
	// this is a side scroller so only worry about X axis
	auto& pPos = _player->getComponent<CTransform>().pos;
	float centerX = std::max(_game->window().getSize().x / 2.f, pPos.x);
	sf::View view = _game->window().getView();
	view.setCenter(centerX, _game->window().getSize().y - view.getCenter().y);
	_game->window().setView(view);

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

		// Draw Collision box
		if (_drawAABB && e->hasComponent<CBoundingBox>()) {
			auto box = e->getComponent<CBoundingBox>();
			sf::RectangleShape rect;
			rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
			centerOrigin(rect);
			rect.setPosition(e->getComponent<CTransform>().pos);
			rect.setFillColor(sf::Color(0, 0, 0, 0));
			rect.setOutlineColor(sf::Color{ 0, 255, 0 });
			rect.setOutlineThickness(2.f);
			_game->window().draw(rect);
		}

	}

	// draw grid
	sf::VertexArray lines(sf::Lines);
	sf::Text gridText;
	gridText.setFont(Assets::getInstance().getFont("Arial"));
	gridText.setCharacterSize(10);

	if (_drawGrid) {
		float left = view.getCenter().x - view.getSize().x / 2.f;
		float right = left + view.getSize().x;
		float top = view.getCenter().y - view.getSize().y / 2.f;
		float bot = top + view.getSize().y;

		// aling grid to grid size
		int nCols = static_cast<int>(view.getSize().x) / _gridCellSize.x;
		int nRows = static_cast<int>(view.getSize().y) / _gridCellSize.y;


		// row and col # of bot left
		const int ROW0 = 768;
		int firstCol = static_cast<int>(left) / static_cast<int>(_gridCellSize.x);
		int lastRow = static_cast<int>(bot) / static_cast<int>(_gridCellSize.y);

		lines.clear();

		// vertical lines

		for (int x{ 0 }; x <= nCols; ++x) {
			lines.append(sf::Vector2f((firstCol + x) * _gridCellSize.x, top));
			lines.append(sf::Vector2f((firstCol + x) * _gridCellSize.x, bot));
		}


		// horizontal lines
		for (int y{ 0 }; y <= nRows; ++y) {
			lines.append(sf::Vertex(sf::Vector2f(left, ROW0 - (lastRow - y) * _gridCellSize.y)));
			lines.append(sf::Vertex(sf::Vector2f(right, ROW0 - (lastRow - y) * _gridCellSize.y)));
		}


		// grid coordinates
		// (firstCol, lastRow) is the bottom left corner of the view
		for (int x{ 0 }; x <= nCols; ++x) {
			for (int y{ 0 }; y <= nRows; ++y) {
				std::string label = std::string(
					"(" + std::to_string(firstCol + x) + ", " + std::to_string(lastRow - y) + ")");
				gridText.setString(label);
				gridText.setPosition((x + firstCol) * _gridCellSize.x, ROW0 - (lastRow - y + 1) * _gridCellSize.y);
				_game->window().draw(gridText);
			}
		}


		_game->window().draw(lines);
	}
}
