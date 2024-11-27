#pragma once

#include "Entity.h"
#include "EntityManager.h"

#include <cmath>
#include <random>

#include <SFML/Graphics.hpp>

struct PlayerConifg { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig  { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConifg { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };
struct WindowConfig { int width, height, framerate, screen; };
struct FontConfig	{ std::string fontPath; int fontSize, r, g, b; };

class Game
{
	std::mt19937 rng;
	sf::RenderWindow m_window;
	EntityManager m_entities;
	sf::Font m_font;
	sf::Text m_text;
	FontConfig m_fontConfig;
	WindowConfig m_windowConfig;
	PlayerConifg m_playerConfig;
	EnemyConfig m_enemyConfig;
	BulletConifg m_bulletConfig;
	int m_score = 0;
	int m_currentFrame = 0;
	int m_lastEnemySpawnTime = 0;
	bool m_paused = false;
	bool m_running = true;

	std::shared_ptr<Entity> m_player;

	void init(const std::string& config);
	void setPauased();
	void resumeGame();

	void sMovement();
	void sUserInput();
	void sLifespan();
	void sRender();
	void sEnemySpawner();
	void sCollision();

	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity);

	template<typename T, typename U> //random number generator
	auto rG(T min, U max)
	{
		using result_t = decltype(min + max);

		if constexpr (std::is_integral_v<result_t>) {
			return std::uniform_int_distribution<result_t>(min, max)(rng);
		}
		else {
			return std::uniform_real_distribution<result_t>(min, max)(rng);
		}
	} 

public:
	Game(const std::string& config);

	void run();
};
