#pragma once
#include <SFML/Graphics.hpp>

namespace conf
{
	sf::Vector2u window_size = { 1920, 1080 };
	sf::Vector2f window_size_f = 
	{ 
		static_cast<float>(window_size.x), 
		static_cast<float>(window_size.y) 
	};
	uint32_t const max_FPS = 144;
	float const dt = 1.f / static_cast <float>(max_FPS);
	sf::Font font("src/fonts/SpaceMono-Regular.ttf");
	sf::Font scoreFont("src/fonts/alarm clock.ttf");
	sf::Font playerFont("src/fonts/Minecraft.ttf");
	sf::Vector2f MinBorder =
	{
		0.f,
		0.f
	};
	sf::Vector2f MaxBorder =
	{
		10000.f,
		1180.f
	};
	sf::Vector2f MaxSize =
	{
		// 3000 + 100 + 1920 = 5020
		MaxBorder.x - (MinBorder.x) + window_size_f.x, // Total 
		// 1080 + 100 + 1080 = 2260
		MaxBorder.y - (MinBorder.y) + window_size_f.y  // - 740
	};
	sf::Vector2f resetPos =
	{ 
		0.f, 
		conf::window_size_f.y - 100.f 
	};
	float Ground = (float)window_size.y;
	float gravity = 981.f;         // pixels per second squared (adjust as needed)
	float jumpVelocity = -600.f;   // negative because y-axis points down
	float moveSpeed = 10.f; // pixels per second
	constexpr float CameraSmoothness = 5.f;
	std::string ScoresFile = "src/data/Scores.json";
}