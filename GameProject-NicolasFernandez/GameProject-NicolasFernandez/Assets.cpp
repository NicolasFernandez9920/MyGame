//
// Created by David Burchill on 2023-10-31.
//

#include "Assets.h"
//#include "MusicPlayer.h"
#include <iostream>
#include <cassert>
#include <fstream>

Assets::Assets()
{}

Assets& Assets::getInstance() {
    static Assets instance;          // Meyers Singleton implementation
    return instance;
}

void Assets::addFont(const std::string& fontName, const std::string& path) {
    std::unique_ptr<sf::Font> font(new sf::Font);
    if (!font->loadFromFile(path))
        throw std::runtime_error("Load failed - " + path);

    auto rc = _fontMap.insert(std::make_pair(fontName, std::move(font)));
    if (!rc.second) assert(0); // big problems if insert fails

    std::cout << "Loaded font: " << path << std::endl;
}

void Assets::addSound(const std::string& soundName, const std::string& path) {
    std::unique_ptr<sf::SoundBuffer> sb(new sf::SoundBuffer);
    if (!sb->loadFromFile(path))
        throw std::runtime_error("Load failed - " + path);

    auto rc = _soundEffects.insert(std::make_pair(soundName, std::move(sb)));
    if (!rc.second) assert(0); // big problems if insert fails

    std::cout << "Loaded sound effect: " << path << std::endl;
}

void Assets::addTexture(const std::string& textureName, const std::string& path, bool smooth)
{
    _textures[textureName] = sf::Texture();
    if (!_textures[textureName].loadFromFile(path)) {
        std::cerr << "Could not load texture file: " << path << std::endl;
        _textures.erase(textureName);
    }
    else {
        _textures.at(textureName).setSmooth(smooth);
        std::cout << "Loaded texture: " << path << std::endl;
    }
}

void Assets::addSpriteRec(const std::string& name, SpriteRec sr)
{
    _spriteRecs[name] = sr;
}

void Assets::addAnimationRec(const std::string& name, AnimationRec ar)
{
    _animationRecs[name] = ar;
}

const sf::Font& Assets::getFont(const std::string& fontName) const {
    auto found = _fontMap.find(fontName);
    assert(found != _fontMap.end());
    return *found->second;
}

const sf::SoundBuffer& Assets::getSound(const std::string& soundName) const {
    auto found = _soundEffects.find(soundName);
    assert(found != _soundEffects.end());
    return *found->second;
}

const sf::Texture& Assets::getTexture(const std::string& textureName) const
{
    if (_textures.find(textureName) == _textures.end()) {
        std::cerr << "Texture not found " << textureName;
        throw std::out_of_range("Texture not found: " + textureName);
    }
    return _textures.at(textureName);
}

const SpriteRec& Assets::getSpriteRec(const std::string& name) const
{
    return _spriteRecs.at(name);
}

const AnimationRec& Assets::getAnimationRec(const std::string& name) const
{
    return _animationRecs.at(name);
}

bool Assets::hasTexture(const std::string& textureName) const
{
    return _textures.find(textureName) != _textures.end();
}


void Assets::loadFonts(const std::string& path)
{
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile) {
        if (token == "Font") {
            std::string name, path;
            confFile >> name >> path;
            addFont(name, path);
        }
        else {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}

void Assets::loadTextures(const std::string& path)
{
    // Read Config file
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile) {
        if (token == "Texture") {
            std::string name, path;
            confFile >> name >> path;
            addTexture(name, path);
            std::cout << "Texture loaded: " << name << " from " << path << "\n";
        }
        else {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}

//void Assets::loadSounds(const std::string& path)
//{
//    std::ifstream confFile(path);
//    if (confFile.fail()) {
//        std::cerr << "Open file " << path << " failed\n";
//        confFile.close();
//        exit(1);
//    }
//
//    std::string token{ "" };
//    confFile >> token;
//    while (confFile) {
//        if (token == "Sound") {
//            std::string name, path;
//            confFile >> name >> path;
//            addSound(name, path);
//        }
//        else {
//            // ignore rest of line and continue
//            std::string buffer;
//            std::getline(confFile, buffer);
//        }
//        confFile >> token;
//    }
//    confFile.close();
//}

void Assets::loadSpriteRecs(const std::string& path)
{
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile) {
        if (token == "Sprite") {
            std::string name;
            SpriteRec sr;
            confFile >> name >> sr.texName >> sr.texRect.left
                >> sr.texRect.top >> sr.texRect.width >> sr.texRect.height;
            addSpriteRec(name, sr);
        }
        else {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}

void Assets::loadAnimationRecs(const std::string& path)
{
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile) {
        if (token == "Animation") {
            float duration;
            std::string name;
            AnimationRec ar;
            confFile >> name >> ar.texName >> ar.frameSize.x
                >> ar.frameSize.y >> ar.numbFrames >> duration >> ar.repeat;
            ar.duration = sf::seconds(duration);
            addAnimationRec(name, ar);
        }
        else {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}

void Assets::loadFromFile(const std::string path) {
    loadFonts(path);
    loadTextures(path);
    //loadSounds(path);
    loadSpriteRecs(path);
    loadAnimationRecs(path);
}
