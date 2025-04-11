#pragma once

#include "Scene.h"

struct PlayerConfig
{
	float X{ 0.f }, Y{ 0.f }, CW{ 0.f }, CH{ 0.f };
	float SPEED{ 0.f }, MAXSPEED{ 0.f }, JUMP{ 0.f }, GRAVITY{ 0.f };
	std::string WEAPON;
};

struct EnemyConfig {
	float X, Y, CW, CH, SPEED, JUMP, MAXSPEED, GRAVITY;
};


const float CELL_SIZE = 64.f;

class Scene_Game : public Scene
{
	sPtrEntt                        _player{ nullptr };
	PlayerConfig			    	_playerConfig;
	std::vector<EnemyConfig>		_enemiesConfig;
	sf::View                        _worldView;
	sf::FloatRect                   _worldBounds;
	sf::Vector2f                    _gridCellSize{ 64.f, 64.f };
	std::vector<sf::Vector2f>       _playerJumpPoints;

	sf::Sprite						_counterIcon;
	sf::Text						_counterText;
	sf::Texture						_counterTexture;
	sf::Font						_counterFont;

	int								_counter{ 0 };
	float							_levelCompleteTimer = 0.f;

	bool                            _enemyPlayerCollison{true};
	bool                            _enemyBulletCollison{ true };
	bool                            _drawTextures{ true };
	bool                            _drawAABB{ false };
	bool                            _drawGrid{ false };
	bool							_levelCompleted{ false };


	//systems
	void                    sUpdate(sf::Time dt);
	void                    sMovement();
	void					sCollision();
	void					sLifespan();
	void					sAnimation(sf::Time dt);
	void	                onEnd() override;

	//helper functions
	void                    animatePlayer();
	void                    animateProtester();
	void                    spawnPlayer();
	void					spawnEnemies();
	void					spawnBullet(sPtrEntt entity);
	void                    checkPlayerCollision();
	void                    checkEnemyCollision();
	void					checkIfPlayerIsDead();
	void                    destroyFallenEntities();
	void					enemyCounter();
	void                    playerMovement();
	void					platformMovement();
	void					updateEnemiesMovement();
	void					winning();
	void                    init(const std::string& levelPath);
	void                    loadLevel(const std::string& path);
	void	                registerActions();
	sf::Vector2f            gridToMidPixel(float gridX, float gridY, sPtrEntt entity);

public:
	Scene_Game(GameEngine* gameEngine, const std::string& levelPath);

	void		            update(sf::Time dt) override;
	void		            sDoAction(const Command& command) override;
	void		            sRender() override;
};

