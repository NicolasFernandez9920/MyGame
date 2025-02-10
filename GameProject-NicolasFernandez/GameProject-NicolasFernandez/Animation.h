#pragma once

#include <SFML/Graphics.hpp>
#include <vector>


class Animation {
public:
    std::string                 _name{ "none" };
    std::vector<sf::IntRect>    _frames;
    size_t                      _currentFrame{ 0 };
    sf::Time                    _timePerFrame;
    sf::Time                    _countDown{ sf::Time::Zero };
    bool                        _isRepeating{ true };
    bool                        _hasEnded{ false };
    sf::Sprite                  _sprite;
    sf::Vector2f                _size{ 1.f, 1.f };


public:
    Animation() = default;
    Animation(const std::string& name, const sf::Texture& t,
        std::vector<sf::IntRect> frames, sf::Time tpf, bool repeats = true);

    void                    update(sf::Time dt);
    bool                    hasEnded() const;
    const std::string&      getName() const;
    sf::Sprite&             getSprite();
    sf::Vector2f            getBB() const;
    const sf::Vector2f      getSize() const;
};
