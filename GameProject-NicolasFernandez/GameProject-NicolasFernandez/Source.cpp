////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Assignment:
//  Instructor:     David Burchill
//  Year / Term:    fall 2024
//  File name:      .cpp
//
//  Student name:   Nicolas Fernandez
//  Student email:  rfernandezrios01@mynbcc.ca
//
//     I certify that this work is my work only, any work copied from Stack Overflow, textbooks,
//     or elsewhere is properly cited.
//
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  BUG
//  list any and all bugs in your code:
//
//  1.
//


#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 780), "SFML works!");

    sf::CircleShape shape(100.f);

    shape.setOrigin(100, 100);

    shape.setFillColor(sf::Color::Red);

    shape.setPosition(512, 390);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}
