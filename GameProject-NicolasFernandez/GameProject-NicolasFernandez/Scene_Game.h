#pragma once

#include "Scene.h"

const float CELL_SIZE = 40.f;

class Scene_Game : public Scene
{
	sPtrEntt                        _player{ nullptr };
	sf::View                        _worldView;
	float                           _gridCellSize{ 40.f };

	bool                            _drawTextures{ true };
	bool                            _drawAABB{ false };
	bool                            _drawGrid{ false };

	//systems


	//helper functions
	void                    spawnPlayer(sf::Vector2f pos);
	void                    init(const std::string& path);
	void                    loadLevel(const std::string& path);

public:
	Scene_Game(GameEngine* gameEngine, const std::string& levelPath);

	void		            update(sf::Time dt) override;
	void		            sDoAction(const Command& command) override;
	void		            sRender() override;
};

