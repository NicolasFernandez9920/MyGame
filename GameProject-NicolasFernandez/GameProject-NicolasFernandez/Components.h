//
// Created by David Burchill on 2023-09-27.
//

#ifndef BREAKOUT_COMPONENTS_H
#define BREAKOUT_COMPONENTS_H


#include <memory>
#include <SFML/Graphics.hpp>
#include "Animation.h"
#include "Utilities.h"



struct Component
{
    bool		has{ false };
    Component() = default;
};

struct CSprite : public Component {
    sf::Sprite sprite;

    CSprite() = default;
    CSprite(const sf::Texture& t)
        : sprite(t) {
        centerOrigin(sprite);
    }
    CSprite(const sf::Texture& t, sf::IntRect r)
        : sprite(t, r) {
        centerOrigin(sprite);
    }
};

struct CLifespan : public Component
{
    int total{ 0 };
    int remaining{ 0 };

    CLifespan() = default;
    CLifespan(int t) : total(t), remaining{ t } {}

};


struct CTransform : public Component
{

    sf::Transformable   tfm;
    sf::Vector2f	    pos{ 0.f, 0.f };
    sf::Vector2f	    prevPos{ 0.f, 0.f };
    sf::Vector2f	    vel{ 0.f, 0.f };
    sf::Vector2f	    scale{ 1.f, 1.f };

    float   angVel{ 0 };
    float	angle{ 0.f };

    CTransform() = default;
    CTransform(const sf::Vector2f& p) : pos(p) {}
    CTransform(const sf::Vector2f& p, const sf::Vector2f& v, const sf::Vector2f& sc, float a)
        : pos(p), prevPos(p), vel(v), scale(sc), angle(a) {}

};

struct CCollision : public Component
{
    float radius{ 0.f };

    CCollision() = default;
    CCollision(float r)
        : radius(r) {}
};

struct CAnimation : public Component
{
    Animation       animation;
    sf::Vector2i    frameSize{ 0,0 };
    size_t          numbFrames{ 1 };
    size_t          currentFrame{ 0 };
    sf::Time        timePerFrame{ sf::Time::Zero };
    sf::Time        countDown{ sf::Time::Zero };
    bool            isRepeat{ true };

    CAnimation() = default;

    inline bool    isFinished() {
        return (!isRepeat && currentFrame >= numbFrames);
    }
};


struct CBoundingBox : public Component
{
    sf::Vector2f size{ 0.f, 0.f };
    sf::Vector2f halfSize{ 0.f, 0.f };

    CBoundingBox() = default;
    CBoundingBox(const sf::Vector2f& s) : size(s), halfSize(0.5f * s)
    {}

    CBoundingBox(float w, float h) : size(sf::Vector2f(w, h)), halfSize(0.5f * size)
    {}

    CBoundingBox(const sf::Vector2u& s) : size(static_cast<float>(s.x), static_cast<float>(s.y)),
        halfSize(0.5f * size)
    {}
};



struct CInput : public Component
{
    bool up{ false };
    bool left{ false };
    bool right{ false };
    bool down{ false };
    bool shoot{ false };
    bool canShoot{ true };
    bool canJump{ true };
    bool hasWeapon{ false };

    CInput() = default;
};


struct CScore : public Component
{
    int score{ 0 };
    CScore(int s = 0) : score(s) {}
};


struct CState : public Component {
    enum State {
        isGrounded = 1,
        isFacingLeft = 1 << 1,
        isRunning = 1 << 2
    };
    unsigned int  state{ 0 };

    CState() = default;
    CState(unsigned int s) : state(s) {}
    bool test(unsigned int x) { return (state & x); }
    void set(unsigned int x) { state |= x; }
    void unSet(unsigned int x) { state &= ~x; }
};

struct CPlayerState : public Component
{
    bool isDead{ false };
    sf::Time respawnTime{ sf::Time::Zero };

    CPlayerState() = default;
};

#endif //BREAKOUT_COMPONENTS_H
