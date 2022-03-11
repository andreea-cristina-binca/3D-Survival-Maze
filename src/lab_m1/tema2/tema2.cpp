#include "lab_m1/tema2/tema2.h"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace m1;


Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
	window->DisablePointer();

	// Get labyrinth
	ReadMap();

	// Initialize player and enemies based on data from the read labyrinth
	ProcessMap();

	// Set camera type
	firstPerson = false;

	// Create camera
	camera = myNamespace::Camera();
	glm::vec3 cameraPos = player.position - glm::vec3(0.0f, 0.0f, 2.0f);
	camera.Set(cameraPos, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Load meshes
	Mesh* mesh = new Mesh("box");
	mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
	meshes[mesh->GetMeshID()] = mesh;

	mesh = new Mesh("sphere");
	mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
	meshes[mesh->GetMeshID()] = mesh;

	// Set projection parameters
	left = -window->GetResolution().x / 300;
	right = window->GetResolution().x / 300;
	bottom = -window->GetResolution().y / 300;
	top = window->GetResolution().y / 300;

	zNearOrtho = 0.1f;
	zFarOrtho = 200.0f;

	fov = RADIANS(60);
	aspect = window->props.aspectRatio;
	zNear = 0.01f;
	zFar = 200.0f;

	// Set gameplay parameters
	healthMax = 3.0f;
	healthScale = 3.0f;
	timeMax = 30.0f;
	timeScale = 30.0f;
	gameOver = false;
	reachEnd = false;

	// Create a shader program for drawing face polygon with the color of the normal
	Shader* shader = new Shader("TemaShader");
	shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
	shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
	shader->CreateAndLink();
	shaders[shader->GetName()] = shader;
}


void Tema2::ReadMap()
{
	std::vector<Labyrinth> labyrinths;

	ifstream in("./src/lab_m1/tema2/labyrinths.txt");

	// Get number of labyrinths to be read
	int num;
	in >> num;

	// Read all labyrinths
	for (int k = 0; k < num; k++) {
		int l, w;
		in >> l >> w;

		// Create new labyrinth
		Labyrinth lab(l, w);

		float x = 0.0f;
		float z = 0.0f;

		// Read lines
		for (int i = 0; i < l; i++) {
			vector<glm::vec4> nextLine;

			// Read column elements
			for (int j = 0; j < w; j++) {
				int val;
				in >> val;

				// Save the position of the current cell and the type of the cell
				glm::vec4 posVal = glm::vec4(x, 0.5f, z, val);

				nextLine.push_back(posVal);

				// Move position for the next cell
				x += 3.0f;
			}

			lab.labyrinth.push_back(nextLine);

			// Adjust positioning
			x = 0.0f;
			z += 3.0f;

		}

		labyrinths.push_back(lab);
	}

	in.close();

	// Get last used labyrinth
	ifstream fin("./src/lab_m1/tema2/generated.txt");
	int last;
	fin >> last;
	fin.close();

	// Choose a different labyrinth
	srand(time(NULL));
	int value = last;
	while (value == last)
		value = rand() % num;

	// Save the selected labyrinth
	map = labyrinths.at(value);
	map.cellScale = glm::vec3(3.0f);

	// Write the new labyrinth value
	ofstream fout("./src/lab_m1/tema2/generated.txt");
	fout << value;
	fout.close();
}


void Tema2::ProcessMap()
{
	for (int i = 0; i < map.length; i++) {
		for (int j = 0; j < map.width; j++) {
			glm::vec4 cell = map.labyrinth.at(i).at(j);

			// Get spawn point for the player
			if (cell.w == 3) {
				player.position = glm::vec3(cell.x, 1, cell.z);
				player.rotation = 0.0f;
				player.speed = 10.0f;
				player.scale = glm::vec3(0.5f);
				player.actualSize = glm::vec3(0.75f, 1.5f, 0.75f);
			}

			// Add a new enemy
			if (cell.w == 2) {
				glm::vec3 enemyPos = glm::vec3(cell.x, cell.y, cell.z);
				float speed = 1.0f;
				glm::vec3 enemyScale = glm::vec3(1.5f, 1.0f, 1.5f);

				Enemy enemy(enemyPos, speed, enemyScale);

				enemies.push_back(enemy);
			}

			// Add the cells of the outside edges (places where the exit could lead to)
			if (i == 0)
				map.winCells.push_back(glm::vec3(cell.x, cell.y, cell.z - 3.0f));
			if (i == map.length - 1)
				map.winCells.push_back(glm::vec3(cell.x, cell.y, cell.z + 3.0f));
			if (j == 0)
				map.winCells.push_back(glm::vec3(cell.x - 3.0f, cell.y, cell.z));
			if (j == map.width - 1)
				map.winCells.push_back(glm::vec3(cell.x + 3.0f, cell.y, cell.z));
		}
	}
}


void Tema2::FrameStart()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();

	// Sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}


void Tema2::Update(float deltaTimeSeconds)
{
	window->CenterPointer();

	// Reduce timer
	timeScale -= deltaTimeSeconds;

	// Get projection and view matrix
	projectionMatrix = glm::perspective(fov, aspect, zNear, zFar);
	viewMatrix = camera.GetViewMatrix();

	// Render map
	RenderMap();

	// Render player
	if (!firstPerson) {
		RenderPlayer();
		camera.Set(camera.position, player.position, glm::vec3(0, 1, 0));
	}

	// Update the bullets and render them
	BulletUpdate(deltaTimeSeconds);
	RenderBullet();

	// Update enemies and render them
	EnemyUpdate(deltaTimeSeconds);
	RenderEnemy();

	// Render the interface
	if (!firstPerson) {
		projectionMatrix = glm::ortho(left, right, bottom, top, zNearOrtho, zFarOrtho);
		RenderHUD();
		projectionMatrix = glm::perspective(fov, aspect, zNear, zFar);
	}

	// Verify if the game is over
	CheckWin();
	VerifyGameState();

	if (gameOver) {
		window->Close();
	}
}


void Tema2::BulletUpdate(float deltaTimeSeconds)
{
	// Calculate projectiles trajectories
	auto it = bullets.begin();

	while (it != bullets.end()) {
		bool go = true;

		if (it->lifetime > 0) {
			// Calculate new position
			it->position += it->direction * it->speed;

			// Reduce time until the bullet disappears
			it->lifetime -= deltaTimeSeconds * 100;

			// Check collision with enemies
			auto enemy = enemies.begin();

			while (enemy != enemies.end()) {
				if (CheckCollision(enemy->position, glm::vec3(enemy->scale.x, enemy->scale.y * 2, enemy->scale.z), it->position, it->scale)) {
					// Delete enemy
					enemy = enemies.erase(enemy);

					// Delete bullet
					go = false;
				}
				else
					enemy++;
			}

			// Check collision with walls
			for (int i = 0; i < map.length; i++) {
				for (int j = 0; j < map.width; j++) {
					glm::vec4 cell = map.labyrinth.at(i).at(j);
					glm::vec3 cellPosition = glm::vec3(cell.x, cell.y, cell.z);

					if ((int)cell.w == 1)
						if (CheckCollision(cellPosition, map.cellScale, it->position, it->scale))
							// Delete bullet
							go = false;
				}
			}
		}
		else
			go = false;

		if (go)
			it++;
		else
			it = bullets.erase(it);
	}
}


void Tema2::EnemyUpdate(float deltaTimeSeconds)
{
	auto it = enemies.begin();

	while (it != enemies.end()) {
		//Update position
		it->position.y += deltaTimeSeconds * it->speed;

		if (it->position.y > 2.0f || it->position.y < 0.5f) {
			it->speed = -it->speed;
			it->position.y += deltaTimeSeconds * it->speed;
		}

		it++;
	}
}


void Tema2::CheckWin()
{
	for (glm::vec3 cell : map.winCells) {
		if (CheckCollision(cell, map.cellScale, player.position, player.actualSize)) {
			reachEnd = true;
			break;
		}
	}
}


void Tema2::VerifyGameState()
{
	if (healthScale <= 0.0f || timeScale <= 0.0f) {
		cout << "-------------" << endl;
		cout << "| YOU LOST! |" << endl;
		cout << "| GAME OVER |" << endl;
		cout << "-------------" << endl;
		gameOver = true;
	}

	if (reachEnd) {
		cout << "------------" << endl;
		cout << "| YOU WON! |" << endl;
		cout << "| GOOD JOB |" << endl;
		cout << "------------" << endl;
		gameOver = true;
	}
}


void Tema2::FrameEnd()
{
	DrawCoordinateSystem(camera.GetViewMatrix(), projectionMatrix);
}


void Tema2::RenderMap()
{
	// Render floor
	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(map.length * 1.35f, 0.0f, map.width * 1.35f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(map.length * 3, 0.03f, map.width * 3));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(1.0f, 0.9f, 0.3f));

	modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(map.length * 1.35f, 0.0f, map.width * 1.35f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(map.length * 4.0f, 0.02f, map.width * 4.0f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.9f, 0.2f, 0.0f));

	// Render labyrinth
	for (int i = 0; i < map.length; i++) {
		for (int j = 0; j < map.width; j++) {
			// Get data from current cell
			glm::vec4 cell = map.labyrinth.at(i).at(j);

			// Render wall
			if ((int)cell.w == 1) {
				modelMatrix = glm::mat4(1);
				modelMatrix = glm::translate(modelMatrix, glm::vec3(cell.x, cell.y + 1, cell.z));
				modelMatrix = glm::scale(modelMatrix, map.cellScale);
				RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.3f, 0.8f, 0.6f));
			}
		}
	}
}


void Tema2::RenderPlayer()
{
	glm::mat4 relativeMatrix;

	// Render body
	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, player.position);
	modelMatrix = glm::rotate(modelMatrix, player.rotation, glm::vec3(0, player.position.y, 0));
	relativeMatrix = modelMatrix;
	modelMatrix = glm::scale(modelMatrix, glm::vec3(player.scale.x, player.scale.y + 0.25f, player.scale.z - 0.2f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.2f, 0.4f, 0.8f));

	// Render head
	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.52f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3f, 0.25f, 0.3f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(1.0f, 0.8f, 0.6f));

	// Render left arm
	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.4f, 0.15f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.45f, 0.3f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.2f, 0.4f, 0.8f));

	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.4f, -0.16f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.15f, 0.3f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(1.0f, 0.8f, 0.6f));

	// Render right arm
	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.4f, 0.15f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.45f, 0.3f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.2f, 0.4f, 0.8f));

	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.4f, -0.16f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.15f, 0.3f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(1.0f, 0.8f, 0.6f));

	// Render left leg
	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.15f, -0.7f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.6f, 0.3f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.2f, 0.2f, 0.5f));

	// Render right leg
	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.15f, -0.7f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.6f, 0.3f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.2f, 0.2f, 0.5f));
}


void Tema2::RenderBullet()
{
	for (Bullet bullet : bullets) {
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, bullet.position);
		modelMatrix = glm::scale(modelMatrix, bullet.scale);
		RenderMesh(meshes["sphere"], shaders["TemaShader"], modelMatrix, glm::vec3(0.4f, 0.5f, 0.6f));
	}
}


void Tema2::RenderEnemy()
{
	for (Enemy enemy : enemies) {
		// Render enemy body
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(enemy.position));
		modelMatrix = glm::scale(modelMatrix, enemy.scale);
		RenderMesh(meshes["sphere"], shaders["TemaShader"], modelMatrix, glm::vec3(0.4f, 0.0f, 0.4f));

		modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.6f, 0));
		RenderMesh(meshes["sphere"], shaders["TemaShader"], modelMatrix, glm::vec3(0.8f, 0.0f, 0.8f));
	}
}


void Tema2::RenderHUD()
{
	glm::mat4 relativeMatrix;

	// Calculate player position
	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, player.position);
	modelMatrix = glm::rotate(modelMatrix, player.rotation, glm::vec3(0, player.position.y, 0));
	relativeMatrix = modelMatrix;

	// Render health bar relative at the player
	modelMatrix = glm::translate(modelMatrix, glm::vec3(2 + (healthMax - healthScale) / 2, 1.5f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(healthScale, 0.25f, 0.25f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.8f, 0.0f, 0.2f));

	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(2.0f, 1.5f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(healthMax, 0.25f, 0.25f));
	meshes["box"]->SetDrawMode(GL_LINE_STRIP);
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.8f, 0.0f, 0.2f));
	meshes["box"]->SetDrawMode(GL_TRIANGLES);

	// Render time bar
	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-2 - (timeMax - timeScale) / 20, 1.5f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(timeScale / 10, 0.25f, 0.25f));
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.2f, 0.0f, 0.8f));

	modelMatrix = relativeMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 1.5f, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(timeMax / 10, 0.25f, 0.25f));
	meshes["box"]->SetDrawMode(GL_LINE_STRIP);
	RenderMesh(meshes["box"], shaders["TemaShader"], modelMatrix, glm::vec3(0.2f, 0.0f, 0.8f));
	meshes["box"]->SetDrawMode(GL_TRIANGLES);
}


void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// Render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	GLint object_color = glGetUniformLocation(shader->program, "object_color");
	glUniform3fv(object_color, 1, glm::value_ptr(color));

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->m_VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


bool Tema2::CheckCollision(glm::vec3 aPosition, glm::vec3 aScale, glm::vec3 bPosition, glm::vec3 bScale)
{
	if ((fabs(aPosition.x - bPosition.x) < aScale.x / 2 + bScale.x / 2) &&
		(fabs(aPosition.y - bPosition.y) < aScale.y / 2 + bScale.y / 2) &&
		(fabs(aPosition.z - bPosition.z) < aScale.z / 2 + bScale.z / 2))
		return true;
	return false;
}


bool Tema2::WallCollision()
{
	// Check collision with labyrinth walls
	for (int i = 0; i < map.length; i++) {
		for (int j = 0; j < map.width; j++) {
			glm::vec4 cell = map.labyrinth.at(i).at(j);
			glm::vec3 cellPosition = glm::vec3(cell.x, cell.y, cell.z);

			if ((int)cell.w == 1)
				if (CheckCollision(cellPosition, map.cellScale, player.position, player.actualSize))
					return true;
		}
	}

	return false;
}


bool Tema2::EnemyCollision(float deltaTime)
{
	for (Enemy enemy : enemies) {
		if (CheckCollision(enemy.position, enemy.scale + 0.2f, player.position, player.actualSize)) {
			healthScale -= deltaTime;
			return true;
		}
	}

	return false;
}


void Tema2::PlayerMoveUp(float deltaTime)
{
	bool wallCollision = false;
	bool enemyCollision = false;

	glm::vec3 dir = glm::normalize(glm::vec3(camera.forward.x, 0, camera.forward.z));
	player.position += dir * player.speed * deltaTime;

	// Check for collisions
	wallCollision = WallCollision();
	enemyCollision = EnemyCollision(deltaTime);

	player.position -= dir * player.speed * deltaTime;

	// Translate the camera forward and move player forward
	if (!wallCollision && !enemyCollision) {
		camera.MoveForward(player.speed * deltaTime);
		player.position += dir * player.speed * deltaTime;
	}
}


void Tema2::PlayerMoveDown(float deltaTime)
{
	bool wallCollision = false;
	bool enemyCollision = false;

	glm::vec3 dir = glm::normalize(glm::vec3(camera.forward.x, 0, camera.forward.z));
	player.position -= dir * player.speed * deltaTime;

	// Check for collisions
	wallCollision = WallCollision();
	enemyCollision = EnemyCollision(deltaTime);

	player.position += dir * player.speed * deltaTime;

	// Translate the camera backward and move player backward
	if (!wallCollision && !enemyCollision) {
		camera.MoveForward(-player.speed * deltaTime);
		player.position -= dir * player.speed * deltaTime;
	}
}


void Tema2::PlayerMoveLeft(float deltaTime)
{
	bool wallCollision = false;
	bool enemyCollision = false;

	player.position -= glm::normalize(camera.right) * player.speed * deltaTime;

	// Check for collisions
	wallCollision = WallCollision();
	enemyCollision = EnemyCollision(deltaTime);

	player.position += glm::normalize(camera.right) * player.speed * deltaTime;

	// Translate the camera to the left and move player left
	if (!wallCollision && !enemyCollision) {
		camera.TranslateRight(-player.speed * deltaTime);
		player.position -= glm::normalize(camera.right) * player.speed * deltaTime;
	}
}


void Tema2::PlayerMoveRight(float deltaTime)
{
	bool wallCollision = false;
	bool enemyCollision = false;

	player.position += glm::normalize(camera.right) * player.speed * deltaTime;

	// Check for collisions
	wallCollision = WallCollision();
	enemyCollision = EnemyCollision(deltaTime);

	player.position -= glm::normalize(camera.right) * player.speed * deltaTime;

	// Translate the camera to the right  and move player right
	if (!wallCollision && !enemyCollision) {
		camera.TranslateRight(player.speed * deltaTime);
		player.position += glm::normalize(camera.right) * player.speed * deltaTime;
	}
}


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	// Player movement
	if (!firstPerson) {
		if (window->KeyHold(GLFW_KEY_W)) {
			PlayerMoveUp(deltaTime);
		}

		if (window->KeyHold(GLFW_KEY_A)) {
			PlayerMoveLeft(deltaTime);
		}

		if (window->KeyHold(GLFW_KEY_S)) {
			PlayerMoveDown(deltaTime);
		}

		if (window->KeyHold(GLFW_KEY_D)) {
			PlayerMoveRight(deltaTime);
		}
	}

	// Camera type
	if (window->KeyHold(GLFW_KEY_LEFT_CONTROL))
		firstPerson = true;
	else
		firstPerson = false;
}


void Tema2::OnKeyPress(int key, int mods)
{
	if (key == GLFW_KEY_SPACE && window->KeyHold(GLFW_KEY_LEFT_CONTROL)) {
		// Create new bullet
		Bullet bullet;
		bullet.position = camera.position;
		bullet.speed = 0.5f;
		bullet.lifetime = 20.0f;
		bullet.direction = camera.forward;
		bullet.scale = glm::vec3(0.25f);

		// Add the new bullet
		bullets.push_back(bullet);
	}
}


void Tema2::OnKeyRelease(int key, int mods)
{
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	float sensivityOX = 0.001f;
	float sensivityOY = 0.001f;

	if (firstPerson) {
		// Rotate the camera in first-person mode
		camera.RotateFirstPerson_OX(-sensivityOX * deltaY);
		camera.RotateFirstPerson_OY(-sensivityOY * deltaX);
	}
	else {
		// Rotate the camera in third-person mode
		camera.RotateThirdPerson_OX(-sensivityOX * deltaY);
		camera.RotateThirdPerson_OY(-sensivityOY * deltaX);

		// Change player rotation
		player.rotation -= sensivityOY * deltaX;
	}
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}
