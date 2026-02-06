#include "Scene_Game.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
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
	spawnEnemies();

	_counterTexture.loadFromFile("../assets/textures/enemyRedCapHead.png");
	if (!_counterTexture.loadFromFile("../assets/textures/enemyRedCapHead.png")) {
		std::cout << "texture not found\n";
	}

	_counterFont.loadFromFile("../assets/fonts/PressStart2P-vaV7.ttf");
	if (!_counterFont.loadFromFile("../assets/fonts/PressStart2P-vaV7.ttf")) {
		std::cout << "font not found\n";
	}

	_counterIcon.setTexture(_counterTexture);
	_counterIcon.setPosition(30.f, 30.f);

	_counterText.setFont(_counterFont);
	_counterText.setCharacterSize(40);
	_counterText.setFillColor(sf::Color::White);

	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(5);

}

void Scene_Game::sUpdate(sf::Time dt)
{
	const sf::Time SPF = sf::seconds(1.0f / 60.f);

	if (_isPaused) return;

	if (_levelCompleted) {
		_levelCompleteTimer += SPF.asSeconds();
		return;
	}

	_entityManager.update();

	sAnimation(dt);
	sCollision();
	sMovement();
	sLifespan();
	destroyFallenEntities();
	checkIfPlayerIsDead();
}

void Scene_Game::sMovement()
{
	animatePlayer();
	animateProtester();
	playerMovement();
	updateEnemiesMovement();
	platformMovement();

	auto& pt = _player->getComponent<CTransform>();

	// gravity
	pt.vel.y += _playerConfig.GRAVITY;

	// player movement
	pt.vel.x = pt.vel.x * _playerConfig.SPEED;

	 //gravity for enemies
		for (auto e : _entityManager.getEntities("protester")) {
			auto& tfm = e->getComponent<CTransform>();
			tfm.vel.y += _enemiesConfig[0].GRAVITY;
		}


	// move all entities
	for (auto e : _entityManager.getEntities()) {
		auto& tx = e->getComponent<CTransform>();
		tx.prevPos = tx.pos;
		tx.pos += tx.vel;
	}
}



void Scene_Game::sCollision()
{
	checkPlayerCollision();
	checkEnemyCollision();
}

void Scene_Game::sLifespan()
{
	// move all entities
	for (auto e : _entityManager.getEntities("bullet")) {
		auto& lifespan = e->getComponent<CLifespan>();
		if (lifespan.has) {
			lifespan.remaining -= 1;
			if (lifespan.remaining < 0) {
				e->getComponent<CLifespan>().has = false;
				e->destroy();
			}
		}
	}
}

void Scene_Game::sAnimation(sf::Time dt)
{
	for (auto e : _entityManager.getEntities()) {
		// update all animations
		if (e->getComponent<CAnimation>().has) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);
		}
	}
}

void Scene_Game::animatePlayer()
{
	auto& sprite = _player->getComponent<CSprite>().sprite;
	auto pv = _player->getComponent<CTransform>().vel;

	if (pv.x < -0.1) {
		sprite.setScale(-1.f, 1.f);
		_player->getComponent<CState>().set(CState::isFacingLeft);

	}
	else if (pv.x > 0.1) {
		sprite.setScale(1.f, 1.f);
		_player->getComponent<CState>().unSet(CState::isFacingLeft);
	}

}

void Scene_Game::animateProtester()
{
	for (auto e : _entityManager.getEntities("protester")) {

		auto& sprite = e->getComponent<CSprite>().sprite;
		auto ev = e->getComponent<CTransform>().vel;

		if (ev.x < -0.1) {
			sprite.setScale(1.f, 1.f);
		}
		else if (ev.x > 0.1) {
			sprite.setScale(-1.f, 1.f);
		}

	}
}

void Scene_Game::onEnd()
{
}


void Scene_Game::spawnPlayer()
{
	_player = _entityManager.addEntity("player");

	auto& sr = Assets::getInstance().getSpriteRec("playerDT");
	auto& sprite = _player->addComponent<CSprite>(Assets::getInstance().getTexture(sr.texName)).sprite;
	sprite.setTextureRect(sr.texRect);
	centerOrigin(sprite);

	_player->addComponent<CBoundingBox>(sf::Vector2f(_playerConfig.CW, _playerConfig.CH));
	_player->addComponent<CTransform>(gridToMidPixel(_playerConfig.X, _playerConfig.Y, _player));
	_player->addComponent<CState>();
	_player->addComponent<CInput>();
	_player->addComponent<CPlayerState>();
}

void Scene_Game::spawnEnemies()
{

	for (const auto& enemyConfig : _enemiesConfig) {

		auto enemy = _entityManager.addEntity("protester");

		auto& sr = Assets::getInstance().getSpriteRec("protesterEnemy");
		auto& sprite = enemy->addComponent<CSprite>(Assets::getInstance().getTexture(sr.texName)).sprite;
		sprite.setTextureRect(sr.texRect);
		centerOrigin(sprite);

		enemy->addComponent<CBoundingBox>(sf::Vector2f(enemyConfig.CW, enemyConfig.CH));
		auto& tfm = enemy->addComponent<CTransform>(gridToMidPixel(enemyConfig.X, enemyConfig.Y, enemy));
		tfm.vel.x = enemyConfig.SPEED;
		enemy->addComponent<CEnemyState>().isDefeated = false;
		enemy->addComponent<CInput>();
	}
}


void Scene_Game::spawnBullet(sPtrEntt e)
{
	auto tx = e->getComponent<CTransform>();

	if (tx.has) {
		auto bullet = _entityManager.addEntity("bullet");

		bullet->addComponent<CSprite>(Assets::getInstance().getTexture(_playerConfig.WEAPON)).sprite;
		bullet->addComponent<CBoundingBox>(Assets::getInstance().getTexture(_playerConfig.WEAPON).getSize());
		bullet->addComponent<CLifespan>(60);
		bullet->addComponent<CTransform>(tx.pos);
		bullet->getComponent<CTransform>().vel.x = 10 * (e->getComponent<CState>().test(CState::isFacingLeft) ? -1 : 1);
		bullet->getComponent<CTransform>().vel.y = 0;
	}
}

void Scene_Game::checkPlayerCollision()
{
	// player with collidable objects
	auto players = _entityManager.getEntities("player");

	std::vector<std::shared_ptr<Entity>> collidableObjects = _entityManager.getEntities("tile");

	auto walls = _entityManager.getEntities("wall");
	auto platform = _entityManager.getEntities("platform");

	collidableObjects.insert(collidableObjects.end(), walls.begin(), walls.end());
	collidableObjects.insert(collidableObjects.end(), platform.begin(), platform.end());

	for (auto p : players) {
		p->getComponent<CState>().unSet(CState::isGrounded); // not grounded
		for (auto obj : collidableObjects) {
			auto overlap = Physics::getOverlap(p, obj);
			if (overlap.x > 0 && overlap.y > 0) // +ve overlap in both x and y means collision
			{
				auto prevOverlap = Physics::getPreviousOverlap(p, obj);
				auto& ptx = p->getComponent<CTransform>();
				auto otx = obj->getComponent<CTransform>();


				// collision is in the y direction
				if (prevOverlap.x > 0) {
					if (ptx.prevPos.y < otx.prevPos.y) {
						// player standing on something isGrounded
						p->getComponent<CTransform>().pos.y -= overlap.y;
						p->getComponent<CInput>().canJump = true;
						p->getComponent<CState>().set(CState::isGrounded);

						// if object is platfrom, the player moves with it
						if (obj->getTag() == "platform") {
							p->getComponent<CTransform>().pos.y += otx.vel.y;
						}
					}
					else {
						// player hit something from below
						p->getComponent<CTransform>().pos.y += overlap.y;
					}
					p->getComponent<CTransform>().vel.y = 0.f;
				}


				// collision is in the x direction
				if (prevOverlap.y > 0) {
					if (ptx.prevPos.x < otx.prevPos.x) // player left of tile
						p->getComponent<CTransform>().pos.x -= overlap.x;
					else
						p->getComponent<CTransform>().pos.x += overlap.x;
				}
			}
		}
	}

	// Collision with enemy
		for (auto e : _entityManager.getEntities("protester")) {
			if (e->hasComponent<CEnemyState>() && e->getComponent<CEnemyState>().isDefeated)
				continue; // it doesn't kill the player if enemy is defeated

			auto overlap = Physics::getOverlap(_player, e);

			if (overlap.x > 0 && overlap.y > 0) {

				_player->getComponent<CPlayerState>().isDead = true;
				_player->destroy();
			}
		}

		// Collision with spikes
		for (auto s : _entityManager.getEntities("spike")) {

			auto overlap = Physics::getOverlap(_player, s);

			if (overlap.x > 0 && overlap.y > 0) {

				_player->getComponent<CPlayerState>().isDead = true;
				_player->destroy();
			}
		}

		// Collision with door
		// player spawn point 244, 5
		if (_counter >= 1) {
			for (auto d : _entityManager.getEntities("door")) {

				auto overlap = Physics::getOverlap(_player, d);

				if (overlap.x > 0 && overlap.y > 0) {

					_levelCompleted = true;
					_levelCompleteTimer = 0.f;
				}
			}
		}



	// Collision with pickup
	for (auto e : _entityManager.getEntities("object")) {
		auto overlap = Physics::getOverlap(_player, e);
		auto& tfm = _player->getComponent<CTransform>();

		if (overlap.x > 0 && overlap.y > 0) {

			// removing player sprite
			_player->removeComponent<CSprite>();

			// adding new sprite
			auto& sr = Assets::getInstance().getSpriteRec("DTRedCap");
			auto& sprite = _player->addComponent<CSprite>(Assets::getInstance().getTexture(sr.texName)).sprite;
			sprite.setTextureRect(sr.texRect);
			centerOrigin(sprite);

			// activate weapon
			_player->getComponent<CInput>().hasWeapon = true;

			// destroy pickup
			e->destroy();

			SoundPlayer::getInstance().play("pickup", tfm.pos);
		}
	}
}

void Scene_Game::checkEnemyCollision()
{
	auto bullets = _entityManager.getEntities("bullet");

	// collision with bullets
		for (auto e : _entityManager.getEntities("protester")) {
			for (auto b : bullets) {
				if (e->hasComponent<CEnemyState>() && e->getComponent<CEnemyState>().isDefeated)
					continue;

				auto overlap = Physics::getOverlap(b, e);

				if (overlap.x > 0 && overlap.y > 0) {

					// removing enemy sprite
					e->removeComponent<CSprite>();

					// adding new sprite
					auto& sr = Assets::getInstance().getSpriteRec("newEnemy");
					auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(sr.texName)).sprite;
					sprite.setTextureRect(sr.texRect);
					centerOrigin(sprite);

					// destroy bullet
					b->destroy();

					if (!e->getComponent<CEnemyState>().isDefeated) {
						_counter++;
						e->getComponent<CEnemyState>().isDefeated = true;
					}

				}
			}
		}

	// collision with hydrants
	auto hydrants = _entityManager.getEntities("wall");

	for (auto e : _entityManager.getEntities("protester")) {
		for (auto h : hydrants) {
			auto overlap = Physics::getOverlap(h, e);


			if (overlap.x > 0 && overlap.y > 0) {
				auto& trf = e->getComponent<CTransform>();

				auto prevOverlap = Physics::getPreviousOverlap(h, e);
				trf.vel.x *= -1;

			}
		}
	}

	// collision with spikes
	auto spikes = _entityManager.getEntities("spike");

	for (auto e : _entityManager.getEntities("protester")) {
		for (auto s : spikes) {
			auto overlap = Physics::getOverlap(s, e);

			if (overlap.x > 0 && overlap.y > 0) {

				e->destroy();
				_counter--;
			}
		}
	}

	// enemy with collidable objects

	std::vector<std::shared_ptr<Entity>> collidableObjects = _entityManager.getEntities("tile");

	auto walls = _entityManager.getEntities("wall");
	auto platform = _entityManager.getEntities("platform");

	collidableObjects.insert(collidableObjects.end(), walls.begin(), walls.end());
	collidableObjects.insert(collidableObjects.end(), platform.begin(), platform.end());

	for (auto e : _entityManager.getEntities("protester")) {
		e->getComponent<CState>().unSet(CState::isGrounded);
		for (auto obj : collidableObjects) {
			auto& tfm = e->getComponent<CTransform>();

			auto overlap = Physics::getOverlap(e, obj);
			if (overlap.x > 0 && overlap.y > 0) // +ve overlap in both x and y means collision
			{
				auto prevOverlap = Physics::getPreviousOverlap(e, obj);
				auto& etx = e->getComponent<CTransform>();
				auto otx = obj->getComponent<CTransform>();


				// collision is in the y direction
				if (prevOverlap.x > 0) {
					if (etx.prevPos.y < otx.prevPos.y) {
						auto& input = e->getComponent<CInput>();

						// enemy standing on something isGrounded
						etx.pos.y -= overlap.y;

						input.canJump = true;

						e->getComponent<CState>().set(CState::isGrounded);

						if (obj->getTag() == "platform") {
							etx.pos.y += otx.vel.y;
						}

					}
					else {
						// enemy hit something from below
						etx.pos.y -= overlap.y;
					}
					e->getComponent<CTransform>().vel.y = 0.f;
				}


				// collision is in the x direction
				if (prevOverlap.y > 0) {
					if (etx.prevPos.x < otx.prevPos.x) // enemy left of tile
						etx.pos.x -= overlap.x;
					else
						etx.pos.x += overlap.x;
				}

			}

		}
	}
}

void Scene_Game::checkIfPlayerIsDead()
{
	if (_player->getComponent<CPlayerState>().isDead) {
		_game->quitLevel();
	}
}


void Scene_Game::destroyFallenEntities()
{
	float bottomY = _worldView.getCenter().y + _worldView.getSize().y / 2.f;

	if (_player && _player->hasComponent<CTransform>()) {
		auto& pos = _player->getComponent<CTransform>().pos;

		if (pos.y > bottomY) {
			_player->getComponent<CPlayerState>().isDead = true;
			_player->destroy();
		}
	}

	for (auto e : _entityManager.getEntities("protester")) {
		if (e->hasComponent<CTransform>()) {
			auto& pos = e->getComponent<CTransform>().pos;

			if (pos.y > bottomY) {
				e->destroy();
				_counter--;
			}
		}
	}
}

void Scene_Game::enemyCounter()
{
	_counterText.setString("X " + std::to_string(_counter) + " / 1");
	_counterText.setPosition(
		_counterIcon.getPosition().x + _counterIcon.getLocalBounds().width + 20.f,
		_counterIcon.getPosition().y + 15.f
	);

	_game->window().draw(_counterIcon);
	_game->window().draw(_counterText);

}

void Scene_Game::playerMovement()
{
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
}

void Scene_Game::platformMovement()
{
	float speed = 2.2f;
	float range = 325.f;

	for (auto e : _entityManager.getEntities("platform")) {
		auto& trf = e->getComponent<CTransform>();

		if (trf.vel.y == 0) {
			trf.vel.y = speed;
		}

		trf.pos.y += trf.vel.y;

		if (trf.pos.y > trf.startPos.y + range || trf.pos.y < trf.startPos.y - range) {
			trf.vel.y *= -1;
		}
	}
}

void Scene_Game::updateEnemiesMovement()
{
	const sf::Time SPF = sf::seconds(1.0f / 60.f);
	auto playerPos = _player->getComponent<CTransform>().pos;
	auto& playerTfm = _player->getComponent<CTransform>();

	for (auto e : _entityManager.getEntities("protester")) {
		auto& tfm = e->getComponent<CTransform>();
		auto& eState = e->getComponent<CEnemyState>();
		auto& input = e->getComponent<CInput>();

		if (eState.isDefeated) {
			float distance = length(playerPos - tfm.pos);


			if (playerTfm.vel.y < 0) {

				static float timeToJump = 0.0f;
				timeToJump += SPF.asSeconds();

				if (timeToJump > 0.35f) {
					if (input.canJump) {
							tfm.vel.y = -_enemiesConfig[0].JUMP;
							input.canJump = false;
							timeToJump = 0.0f;
					}
				}

			}
				// minimum distance between enemy and player
				float minDistance = 1.f;

				// maximum distance between enemy and player
				float maxDistance = 80.f;

				if (distance > maxDistance) {
					sf::Vector2f direction = normalize(playerPos - tfm.pos);
					float speed = 10.f;

					tfm.vel.x = direction.x * speed;
				}
				else if (distance < minDistance) {
					sf::Vector2f direction = normalize(tfm.pos - playerPos);
					float speed = 10.f;

					tfm.vel.x = direction.x * speed;
				}
				else {
					tfm.vel.x = 0.f;
				}
		}
	}
}

void Scene_Game::winning()
{
	const sf::Time SPF = sf::seconds(1.0f / 60.f);

	//
	sf::View currentView = _game->window().getView();
	_game->window().setView(_game->window().getDefaultView());
	//

	sf::RectangleShape fade(sf::Vector2f(_game->window().getSize()));
	fade.setFillColor(sf::Color(0, 0, 0, std::min(200.f, _levelCompleteTimer * 100)));
	_game->window().draw(fade);

	if (_levelCompleteTimer > 1.f) {
		sf::Text text;
		text.setFont(_counterFont);
		text.setCharacterSize(55);
		text.setFillColor(sf::Color::White);
		text.setString("LEVEL COMPLETE!");
		centerOrigin(text);
		text.setPosition(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f - 40);
		_game->window().draw(text);

		sf::Text menu;
		menu.setFont(_counterFont);
		menu.setCharacterSize(35);
		menu.setFillColor(sf::Color::White);
		menu.setString("Press Enter to go to Menu\n\nPress Esc to Quit");
		centerOrigin(menu);
		menu.setPosition(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f + 100);
		_game->window().draw(menu);

		//
		_game->window().setView(currentView);
	}
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
			sprite.setOrigin(0.f, 530.f);
			sprite.setPosition(pos);
		}
		else if (token == "Tile") {
			std::string name;
			float gx, gy;
			config >> name >> gx >> gy;

			// differentiating types of tiles
			std::string entityType = (name == "hydrant") ? "wall" : "tile";

			auto e = _entityManager.addEntity(entityType);
			e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			e->addComponent<CBoundingBox>(Assets::getInstance().getTexture(name).getSize());
			auto pos = gridToMidPixel(gx, gy, e);
			//std::cout << entityType << " tile at (" << pos.x << ", " << pos.y << ")\n";
			e->addComponent<CTransform>(pos);


		}
		else if (token == "Object") {
			std::string name;
			float gx, gy;
			config >> name >> gx >> gy;

			auto e = _entityManager.addEntity("object");
			e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			e->addComponent<CBoundingBox>(Assets::getInstance().getTexture(name).getSize());
			auto pos = gridToMidPixel(gx, gy, e);
			std::cout << "Object at (" << pos.x << ", " << pos.y << ")\n";
			e->addComponent<CTransform>(pos);


		}
		else if (token == "Obstacle") {
			std::string name;
			float gx, gy;
			config >> name >> gx >> gy;

			auto e = _entityManager.addEntity("spike");

			auto texSize = Assets::getInstance().getTexture(name).getSize();
			sf::Vector2f adjustedSize(texSize.x * 0.4f, texSize.y);

			e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			e->addComponent<CBoundingBox>(adjustedSize);
			auto pos = gridToMidPixel(gx, gy, e);
			e->addComponent<CTransform>(pos);

		}
		else if (token == "Lift") {
			std::string name;
			float gx, gy;
			config >> name >> gx >> gy;

			auto e = _entityManager.addEntity("platform");
			e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			e->addComponent<CBoundingBox>(Assets::getInstance().getTexture(name).getSize());
			auto pos = gridToMidPixel(gx, gy, e);
			e->addComponent<CTransform>(pos);

		}
		else if (token == "Dec") {
			std::string name;
			float gx, gy;
			config >> name >> gx >> gy;

			auto e = _entityManager.addEntity("dec");
			e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			auto pos = gridToMidPixel(gx, gy, e);
			e->addComponent<CTransform>(pos);
		}
		else if (token == "End") {
			std::string name;
			float gx, gy;
			config >> name >> gx >> gy;

			auto e = _entityManager.addEntity("door");
			e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			e->addComponent<CBoundingBox>(Assets::getInstance().getTexture(name).getSize());
			auto pos = gridToMidPixel(gx, gy, e);
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
				_playerConfig.GRAVITY >>
				_playerConfig.WEAPON;
		}
		else if (token == "Enemy") {
			EnemyConfig newEnemyConfig{};
			config >>
				newEnemyConfig.X >>
				newEnemyConfig.Y >>
				newEnemyConfig.CW >>
				newEnemyConfig.CH >>
				newEnemyConfig.SPEED >>
				newEnemyConfig.JUMP >>
				newEnemyConfig.MAXSPEED >>
				newEnemyConfig.GRAVITY;

			_enemiesConfig.push_back(newEnemyConfig);
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
	registerAction(sf::Keyboard::Enter, "CONFIRM");
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
	float y = 1080.f - gridY * _gridCellSize.y;

	auto tr = entity->getComponent<CSprite>().sprite.getTextureRect();
	return sf::Vector2f(x + tr.width / 2.f, y - tr.height / 2.f);
}


void Scene_Game::update(sf::Time dt)
{
	sUpdate(dt);
}

void Scene_Game::sDoAction(const Command& command)
{
	if (_levelCompleted) {
		if (command.type() == "START" && command.name() == "CONFIRM" && _levelCompleteTimer >= 2.f) {
			_game->quitLevel();
		}
		return;
	}

	// On Key Press
	if (command.type() == "START") {
		if (command.name() == "PAUSE") { setPaused(!_isPaused); }
		else if (command.name() == "QUIT") { _game->quitLevel(); }
		else if (command.name() == "BACK") { _game->window().close(); }

		else if (command.name() == "TOGGLE_TEXTURE") { _drawTextures = !_drawTextures; }
		else if (command.name() == "TOGGLE_COLLISION") { _drawAABB = !_drawAABB; }
		else if (command.name() == "TOGGLE_GRID") { _drawGrid = !_drawGrid; }

		// Player control
		else if (command.name() == "LEFT") { _player->getComponent<CInput>().left = true; }
		else if (command.name() == "RIGHT") { _player->getComponent<CInput>().right = true; }

		else if (command.name() == "JUMP") {
			auto& input = _player->getComponent<CInput>();
			auto& state = _player->getComponent<CState>();

			if (input.canJump && state.test(CState::isGrounded)) {
				input.up = true;
				input.canJump = false;

				auto& tfm = _player->getComponent<CTransform>();
				//_playerJumpPoints.push_back(tfm.pos);

				//SoundPlayer::getInstance().play("jump", tfm.pos);
			}
		}
		else if (command.name() == "SHOOT") {
			if (_player->getComponent<CInput>().hasWeapon && _player->getComponent<CInput>().canShoot) {
				spawnBullet(_player);
				_player->getComponent<CInput>().shoot = true;
				_player->getComponent<CInput>().canShoot = false;

				auto& tfm = _player->getComponent<CTransform>();

				SoundPlayer::getInstance().play("shoot", tfm.pos);
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
	_game->window().clear(sf::Color(201, 144, 189));

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
		const int ROW0 = 1080;
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

	sf::View worldView = _game->window().getView();
	_game->window().setView(_game->window().getDefaultView());
	enemyCounter();
	_game->window().setView(worldView);

	if (_levelCompleted) {
		winning();
	}

}
