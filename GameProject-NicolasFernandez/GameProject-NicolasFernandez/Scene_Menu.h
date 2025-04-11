#pragma once

#include "Scene.h"

class Scene_Menu : public Scene
{
private:
	std::vector<std::string>	m_menuStrings;
	sf::Text					m_menuText;
	std::vector<std::string>	m_levelPaths;
	int							m_menuIndex{ 0 };
	std::string					m_title;
	sf::Clock					m_blinkClock;
	bool						m_isSubtitleVisible = true;
	bool						m_showConstructionMessage = false;
	bool						m_showInstructions = false;
	std::string					m_message;
	sf::Texture					m_Texture;
	sf::Sprite					m_Sprite;


	void centerTexture(sf::Sprite& sprite);
	void centerText(sf::Text& text);
	void init();
	void onEnd() override;
public:

	Scene_Menu(GameEngine* gameEngine);

	void update(sf::Time dt) override;

	void sRender() override;
	void sDoAction(const Command& action) override;


};


