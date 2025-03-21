#pragma once

#include "Scene.h"

struct PlayerConfig
{
	float X{ 0.f }, Y{ 0.f }, CW{ 0.f }, CH{ 0.f };
	float SPEED{ 0.f }, MAXSPEED{ 0.f }, JUMP{ 0.f }, GRAVITY{ 0.f };
	std::string WEAPON;
};

struct EnemyConfig
{
	float X{ 0.f }, Y{ 0.f }, CW{ 0.f }, CH{ 0.f };
	float SPEED{ 0.f }, MAXSPEED{ 0.f }, JUMP{ 0.f }, GRAVITY{ 0.f };
};

const float CELL_SIZE = 64.f;

class Scene_Game : public Scene
{
	sPtrEntt                        _player{ nullptr };
	PlayerConfig			    	_playerConfig;
	EnemyConfig						_enemyConfig;
	sf::View                        _worldView;
	sf::FloatRect                   _worldBounds;
	sf::Vector2f                    _gridCellSize{ 64.f, 64.f };

	bool                            _drawTextures{ true };
	bool                            _drawAABB{ false };
	bool                            _drawGrid{ false };

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
	void					spawnTweet(sf::Vector2f dir);
	void                    spawnPlayer();
	void					spawnEnemy();
	void					spawnBullet(sPtrEntt entity);
	void                    checkPlayerCollision();
	void                    checkEnemyCollision();
	void                    destroyFallenEntities();
	void                    playerMovement();
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

