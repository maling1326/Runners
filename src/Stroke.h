#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

void ExitStroke(sf::RenderWindow& window);

template<typename T>
void Move(sf::RenderWindow& window, T& object)
{
    float x = object.getPosition().x;
    float y = object.getPosition().y;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        if (x + 10.f < window.getSize().x)
            x += 10.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        if (x - 10.f > 0.f)
            x -= 10.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
    {
        if (y - 10.f > 0.f)
            y -= 10.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
    {
        if (y + 10.f < window.getSize().y)
            y += 10.f;
    }
    object.setPosition({ x, y });
}
