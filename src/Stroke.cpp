#include "Stroke.h"

void ExitStroke(sf::RenderWindow& window)
{
    while (auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }
    }
}

