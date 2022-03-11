#pragma once

#include "components/simple_scene.h"
#include "lab_m1/tema2/camera.h"
#include "lab_m1/tema2/player.h"
#include "lab_m1/tema2/bullet.h"
#include "lab_m1/tema2/labyrinth.h"
#include "lab_m1/tema2/enemy.h"


namespace m1
{
	class Tema2 : public gfxc::SimpleScene
	{
	public:
		Tema2();
		~Tema2();

		void Init() override;

	private:
		void ReadMap();
		void ProcessMap();

		void FrameStart() override;
		void Update(float deltaTimeSeconds) override;
		void BulletUpdate(float deltaTimeSeconds);
		void EnemyUpdate(float deltaTimeSeconds);
		void CheckWin();
		void VerifyGameState();
		void FrameEnd() override;

		void RenderMap();
		void RenderPlayer();
		void RenderBullet();
		void RenderEnemy();
		void RenderHUD();
		void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color);

		bool CheckCollision(glm::vec3 aPosition, glm::vec3 aScale, glm::vec3 bPosition, glm::vec3 bScale);
		bool WallCollision();
		bool EnemyCollision(float deltaTime);
		void PlayerMoveUp(float deltaTime);
		void PlayerMoveDown(float deltaTime);
		void PlayerMoveLeft(float deltaTime);
		void PlayerMoveRight(float deltaTime);

		void OnInputUpdate(float deltaTime, int mods) override;
		void OnKeyPress(int key, int mods) override;
		void OnKeyRelease(int key, int mods) override;
		void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
		void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
		void OnWindowResize(int width, int height) override;

	protected:
		myNamespace::Camera camera;
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		bool firstPerson;

		float left, right, bottom, top;
		float zNearOrtho, zFarOrtho;
		float fov, aspect, zNear, zFar;
		bool projection;

		Player player;
		std::vector<Bullet> bullets;
		Labyrinth map;
		std::vector<Enemy> enemies;

		float healthMax, healthScale;
		float timeMax, timeScale;
		bool gameOver, reachEnd;
	};
}   // namespace m1
