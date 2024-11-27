#include "Game.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>

const float PI = 3.14159265358979323846;

Game::Game(const std::string& config) : rng(std::random_device{}())
{
	init(config);
}

void Game::init(const std::string& path)
{

	std::string temp;

	std::ifstream config("config.txt");

	while (config >> temp)
	{
		if (temp == "Window")
		{
			config >> m_windowConfig.width >> m_windowConfig.height >> m_windowConfig.framerate >> m_windowConfig.screen;
		}
		else if (temp == "Font")
		{

			config >> m_fontConfig.fontPath >> m_fontConfig.fontSize >> m_fontConfig.r >> m_fontConfig.g >> m_fontConfig.b;

			if (!m_font.loadFromFile(m_fontConfig.fontPath))
			{
				std::cerr << "Non è stato possibile caricare il font!\n";
				exit(-1);
			}
		}
		else if (temp == "Player")
		{
			config >> m_playerConfig.SR >> m_playerConfig.CR
				>> m_playerConfig.S
				>> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB
				>> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB
				>> m_playerConfig.OT >> m_playerConfig.V;
		}
		else if (temp == "Enemy")
		{
			config >> m_enemyConfig.SR >> m_enemyConfig.CR
				>> m_enemyConfig.SMIN >> m_enemyConfig.SMAX
				>> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB
				>> m_enemyConfig.VMIN >> m_enemyConfig.VMAX
				>> m_enemyConfig.L >> m_enemyConfig.SI >> m_enemyConfig.OT;
		}
		else if (temp == "Bullet")
		{
			config >> m_bulletConfig.SR >> m_bulletConfig.CR
				>> m_bulletConfig.S
				>> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB
				>> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB
				>> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
		else
		{
			std::cout << "Configurazione non riconosciuta: " << temp << std::endl;
		}
	}

	m_window.create(sf::VideoMode(m_windowConfig.width, m_windowConfig.height), "Titolo");
	m_window.setFramerateLimit(m_windowConfig.framerate);

	m_text.setString("SCORE " + std::to_string(m_score));
	m_text.setFont(m_font);
	m_text.setCharacterSize(m_fontConfig.fontSize);
	m_text.setFillColor(sf::Color(m_fontConfig.r, m_fontConfig.g, m_fontConfig.b));
	m_text.setPosition(20, 20);

	spawnPlayer();
}

void Game::run()
{

	while (m_running)
	{
		m_entities.update();

		sUserInput();

		if (m_paused == false)
		{
			sEnemySpawner();
			sLifespan();
			sMovement();
			sCollision();
			m_currentFrame++;

			
		}
		
		sRender();
		
	}
}

void Game::setPauased()
{
	m_paused = true;
}

void Game::resumeGame()
{
	m_paused = false;
}

void Game::spawnPlayer()
{

	auto player = m_entities.addEntity("player");

	player->cTransform = std::make_shared<CTransform>(
		Vec2(m_windowConfig.width / 2, m_windowConfig.height / 2), 
		Vec2(m_playerConfig.S, m_playerConfig.S), 0.0f);

	player->cShape = std::make_shared<CShape>(
		m_playerConfig.SR, 
		m_playerConfig.V, 
		sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), 
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), 
		m_playerConfig.OT);

	player->cInput = std::make_shared<CInput>();

	player->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);


	m_player = player;
}


void Game::spawnEnemy()
{

	auto vertices = rG(m_enemyConfig.VMIN, m_enemyConfig.VMAX);


	float angle = rG(0.0f, 360.0f);

	float speed = rG(m_enemyConfig.SMIN, m_enemyConfig.SMAX);

	float velocityX = speed * cos(angle * PI / 180.0f);
	float velocityY = speed * sin(angle * PI / 180.0f);

	auto enemy = m_entities.addEntity("enemy");

	enemy->cTransform = std::make_shared<CTransform>(
		Vec2(static_cast<float>(rG(0, m_windowConfig.width)), static_cast<float>(rG(0, m_windowConfig.height))),
		Vec2(velocityX, velocityY),
		angle
	);

	enemy->cShape = std::make_shared<CShape>(
		m_enemyConfig.SR,
		vertices, 
		sf::Color(rG(0, 255), rG(0, 255), rG(0, 255)), 
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), 
		m_enemyConfig.OT
	);

	enemy->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

	enemy->cScore = std::make_shared<CScore>(vertices * 10);

	enemy->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L); 

	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{

	for (int i = 0; i < e->cShape->circle.getPointCount(); i++)
	{
		int numVertices = e->cShape->circle.getPointCount();
		float angleStep = 360.0f / numVertices;

		for (int i = 0; i < numVertices; i++)
		{
			auto smallEnemy = m_entities.addEntity("smallEnemy");

			float angle = i * angleStep;
			float speed = e->cTransform->velocity.length(); 

			float velocityX = speed * cos(angle * PI / 180.0f);
			float velocityY = speed * sin(angle * PI / 180.0f);

			smallEnemy->cTransform = std::make_shared<CTransform>(
				Vec2(e->cTransform->pos.x, e->cTransform->pos.y),
				Vec2(velocityX, velocityY),
				angle);

			smallEnemy->cShape = std::make_shared<CShape>(e->cShape->circle.getRadius() / 2,
				numVertices,
				sf::Color(e->cShape->circle.getFillColor()),
				sf::Color(e->cShape->circle.getOutlineColor()),
				e->cShape->circle.getOutlineThickness());

			smallEnemy->cCollision = std::make_shared<CCollision>(e->cCollision->radius / 2);

			smallEnemy->cScore = std::make_shared<CScore>(numVertices * 20);

			auto test = smallEnemy->cScore;

			smallEnemy->cLifespan = std::make_shared<CLifespan>(100);
		}
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{

	Vec2 direction = target - entity->cTransform->pos;

	Vec2 bulletVelocity = direction.normalized(m_bulletConfig.S);

	auto bullet = m_entities.addEntity("bullet");

	bullet->cTransform = std::make_shared<CTransform>(
		entity->cTransform->pos,  
		bulletVelocity,            
		atan2f(direction.y, direction.x)  
	);

	bullet->cShape = std::make_shared<CShape>(
		m_bulletConfig.SR,
		m_bulletConfig.V,
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
		m_bulletConfig.OT
	);

	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);

	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);



}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{

	auto sword = m_entities.addEntity("sword");

	Vec2 velocity = { 0, 0 };

	float angle = 0;

	float doubleRadius = entity->cShape->circle.getRadius() * 4;

	sword->cTransform = std::make_shared<CTransform>(
		entity->cTransform->pos,
		velocity,
		0
	);

	sword->cShape = std::make_shared<CShape>(
		doubleRadius,
		3,
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB, 100),
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
		m_bulletConfig.OT
	);

	sword->cCollision = std::make_shared<CCollision>(doubleRadius);

	sword->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);


}

void Game::sMovement()
{

	float leftEdgePlayer = m_player->cTransform->pos.x - m_player->cShape->circle.getRadius();
	float rightEdgePlayer = m_player->cTransform->pos.x + m_player->cShape->circle.getRadius();
	float topEdgePlayer = m_player->cTransform->pos.y - m_player->cShape->circle.getRadius();
	float bottomEdgePlayer = m_player->cTransform->pos.y + m_player->cShape->circle.getRadius();

	const float SPEED = m_playerConfig.S;

	m_player->cTransform->velocity = { 0, 0 };

	bool isMoving = false;

	if (m_player->cInput->up)
	{
		m_player->cTransform->velocity.y -= SPEED;
		isMoving = true;

		if (topEdgePlayer < 0)
		{
			m_player->cTransform->velocity.y = 0;
			isMoving = false;

		}

	}

	if (m_player->cInput->down)
	{
		m_player->cTransform->velocity.y += SPEED;
		isMoving = true;

		if (bottomEdgePlayer > m_windowConfig.height)
		{
			m_player->cTransform->velocity.y = 0;
			isMoving = false;

		}
	
	}

	if (m_player->cInput->left)
	{
		m_player->cTransform->velocity.x -= SPEED;
		isMoving = true;

		if (leftEdgePlayer < 0)
		{
			m_player->cTransform->velocity.x = 0;
			isMoving = false;

		}

	}

	if (m_player->cInput->right)
	{
		m_player->cTransform->velocity.x += SPEED;
		isMoving = true;

		if (rightEdgePlayer > m_windowConfig.width)
		{
			m_player->cTransform->velocity.x = 0;
			isMoving = false;
		}

	}

	if (isMoving)
	{
		
		m_player->cTransform->velocity = m_player->cTransform->velocity.normalized(SPEED);

		m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
		m_player->cTransform->pos.y += m_player->cTransform->velocity.y;

	}

	//bullets movements

	for (auto& bullet : m_entities.getEntities("bullet"))
	{
		
		bullet->cTransform->pos.x += bullet->cTransform->velocity.x;
		bullet->cTransform->pos.y += bullet->cTransform->velocity.y;

		
	}

	//enemy movements

	for (auto& enemy : m_entities.getEntities("enemy"))
	{
		enemy->cTransform->pos.x += enemy->cTransform->velocity.x;
		enemy->cTransform->pos.y += enemy->cTransform->velocity.y;


		float radius = enemy->cShape->circle.getRadius();

		float leftEdge = enemy->cTransform->pos.x - radius;
		float rightEdge = enemy->cTransform->pos.x + radius;
		float topEdge = enemy->cTransform->pos.y - radius;
		float bottomEdge = enemy->cTransform->pos.y + radius;

		if (leftEdge < 0) {
			enemy->cTransform->pos.x = radius;
			enemy->cTransform->velocity.x *= -1;
		}
		else if (rightEdge > m_windowConfig.width) 
		{
			enemy->cTransform->pos.x = m_windowConfig.width - radius;
			enemy->cTransform->velocity.x *= -1;
		}

		if (topEdge < 0) 
		{
			enemy->cTransform->pos.y = radius;
			enemy->cTransform->velocity.y *= -1;
		}
		else if (bottomEdge > m_windowConfig.height) 
		{
			enemy->cTransform->pos.y = m_windowConfig.height - radius;
			enemy->cTransform->velocity.y *= -1;
		}

	}

	//small enemy movements

	for (auto& smallEnemy : m_entities.getEntities("smallEnemy"))
	{
		smallEnemy->cTransform->pos.x += smallEnemy->cTransform->velocity.x;
		smallEnemy->cTransform->pos.y += smallEnemy->cTransform->velocity.y;


		float radius = smallEnemy->cShape->circle.getRadius();

		float leftEdge = smallEnemy->cTransform->pos.x - radius;
		float rightEdge = smallEnemy->cTransform->pos.x + radius;
		float topEdge = smallEnemy->cTransform->pos.y - radius;
		float bottomEdge = smallEnemy->cTransform->pos.y + radius;

		if (leftEdge < 0) {
			smallEnemy->cTransform->pos.x = radius;
			smallEnemy->cTransform->velocity.x *= -1;
		}
		else if (rightEdge > m_windowConfig.width)
		{
			smallEnemy->cTransform->pos.x = m_windowConfig.width - radius;
			smallEnemy->cTransform->velocity.x *= -1;
		}

		if (topEdge < 0)
		{
			smallEnemy->cTransform->pos.y = radius;
			smallEnemy->cTransform->velocity.y *= -1;
		}
		else if (bottomEdge > m_windowConfig.height)
		{
			smallEnemy->cTransform->pos.y = m_windowConfig.height - radius;
			smallEnemy->cTransform->velocity.y *= -1;
		}

	}

}

void Game::sLifespan()
{

	for (const auto& entity : m_entities.getEntities())
	{

		if (entity->cLifespan != nullptr && entity->isActive() && (entity->tag() == "bullet" || entity->tag() == "smallEnemy" || entity->tag() == "sword"))
		{
			entity->cLifespan->remaining -= 1;

			float lifePercentage = static_cast<float>(entity->cLifespan->remaining) / entity->cLifespan->total;

			float newAlpha = lifePercentage * 255;

			sf::Color fillColor = entity->cShape->circle.getFillColor();
			fillColor.a = newAlpha;

			sf::Color outlineColor = entity->cShape->circle.getOutlineColor();
			outlineColor.a = newAlpha;

			entity->cShape->circle.setFillColor(fillColor);
			entity->cShape->circle.setOutlineColor(outlineColor);

			if (entity->cLifespan->remaining == 0)
			{
				entity->destroy();
			}
		}

	}

}

void Game::sCollision()
{

	for (const auto& bullet : m_entities.getEntities("bullet"))
	{
		for (const auto& enemy : m_entities.getEntities("enemy"))
		{
			Vec2 differenceBulletAndEnemy = enemy->cTransform->pos - bullet->cTransform->pos;

			float distanceBulletFromEnemy = differenceBulletAndEnemy.length();

			float r1 = bullet->cCollision->radius;
			float r2 = enemy->cCollision->radius;

			if (distanceBulletFromEnemy < r1 + r2)
			{
				bullet->destroy();
				spawnSmallEnemies(enemy);
				enemy->destroy();
				m_score += enemy->cScore->score;
				m_text.setString("SCORE: " + std::to_string(m_score));

			}	

		}
	}

	for (const auto& bullet : m_entities.getEntities("bullet"))
	{
		for (const auto& smallEnemy : m_entities.getEntities("smallEnemy"))
		{
			Vec2 differenceBulletAndSmallEnemy = smallEnemy->cTransform->pos - bullet->cTransform->pos;

			float distanceBulletFromSmallEnemy = differenceBulletAndSmallEnemy.length();

			float r1 = bullet->cCollision->radius;
			float r2 = smallEnemy->cCollision->radius;

			if (distanceBulletFromSmallEnemy < r1 + r2)
			{
				bullet->destroy();
				smallEnemy->destroy();
				m_score += smallEnemy->cScore->score / smallEnemy->cShape->circle.getPointCount();
				m_text.setString("SCORE: " + std::to_string(m_score));

			}

		}
	}

	for (const auto& bullet : m_entities.getEntities("sword"))
	{
		for (const auto& enemy : m_entities.getEntities("enemy"))
		{
			Vec2 differenceBulletAndEnemy = enemy->cTransform->pos - bullet->cTransform->pos;

			float distanceBulletFromEnemy = differenceBulletAndEnemy.length();

			float r1 = bullet->cCollision->radius;
			float r2 = enemy->cCollision->radius;

			if (distanceBulletFromEnemy < r1 + r2)
			{
				spawnSmallEnemies(enemy);
				enemy->destroy();
				m_score += enemy->cScore->score;
				m_text.setString("SCORE: " + std::to_string(m_score));

			}

		}
	}

	for (const auto& bullet : m_entities.getEntities("sword"))
	{
		for (const auto& enemy : m_entities.getEntities("smallEnemy"))
		{
			Vec2 differenceBulletAndEnemy = enemy->cTransform->pos - bullet->cTransform->pos;

			float distanceBulletFromEnemy = differenceBulletAndEnemy.length();

			float r1 = bullet->cCollision->radius;
			float r2 = enemy->cCollision->radius;

			if (distanceBulletFromEnemy < r1 + r2)
			{
				enemy->destroy();
				m_score += enemy->cScore->score / enemy->cShape->circle.getPointCount();
				m_text.setString("SCORE: " + std::to_string(m_score));

			}

		}
	}

	

	for (const auto& enemy : m_entities.getEntities("enemy"))
	{
		Vec2 differencePlayerAndEnemy = enemy->cTransform->pos - m_player->cTransform->pos;

		float distancePlayerFromEnemy = differencePlayerAndEnemy.length();

		float r3 = m_player->cCollision->radius;
		float r4 = enemy->cCollision->radius;

		if (distancePlayerFromEnemy < r3 + r4)
		{
			m_player->destroy();
			enemy->destroy();

			spawnPlayer();

		}

	}


	for (const auto& enemy : m_entities.getEntities("smallEnemy"))
	{
		Vec2 differencePlayerAndEnemy = enemy->cTransform->pos - m_player->cTransform->pos;

		float distancePlayerFromEnemy = differencePlayerAndEnemy.length();

		float r3 = m_player->cCollision->radius;
		float r4 = enemy->cCollision->radius;

		if (distancePlayerFromEnemy < r3 + r4)
		{
			m_player->destroy();
			enemy->destroy();

			spawnPlayer();

		}

	}

}

void Game::sEnemySpawner()
{
	if (m_currentFrame == m_lastEnemySpawnTime + m_enemyConfig.SI)
	{
		spawnEnemy();

	}

}

void Game::sRender()
{

	m_window.clear();

	m_window.draw(m_text);


	for (auto& e : m_entities.getEntities())
	{

		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

		if (e->tag() == "sword")
		{
			e->cTransform->angle += 6.0f;
			e->cShape->circle.setRotation(e->cTransform->angle);
		}
		else
		{
			e->cTransform->angle += 1.0f;
			e->cShape->circle.setRotation(e->cTransform->angle);
		}
		

		m_window.draw(e->cShape->circle);
	}

	m_window.display();
}

void Game::sUserInput()
{

	sf::Event event;
	while (m_window.pollEvent(event))
	{
		
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}


		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{

			case sf::Keyboard::W:
				m_player->cInput->up = true;
				break;

			case sf::Keyboard::A:
				m_player->cInput->left = true;
				break;

			case sf::Keyboard::S:
				m_player->cInput->down = true;
				break;

			case sf::Keyboard::D:
				m_player->cInput->right = true;
				break;

			case sf::Keyboard::X:
				if (!m_paused)
				{
					setPauased();
				}
				else
				{
					resumeGame();
				}
				break;

			default:
				break;

			}
		}

		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{

			case sf::Keyboard::W:
				m_player->cInput->up = false;
				break;

			case sf::Keyboard::A:
				m_player->cInput->left = false;
				break;

			case sf::Keyboard::S:
				m_player->cInput->down = false;
				break;

			case sf::Keyboard::D:
				m_player->cInput->right = false;
				break;

			default:
				break;

			}
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left && m_paused == false)
			{
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.mouseButton.button == sf::Mouse::Right && m_paused == false)
			{
				spawnSpecialWeapon(m_player);
			}
		}
	}
}
