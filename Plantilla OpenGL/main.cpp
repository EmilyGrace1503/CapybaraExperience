#include <glew.h>
#include "Scene.h"
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

//Ventana Principal
GLFWwindow* window;

//Sonidos de Pasos
Music* FXWalk = new Music("Assets/Music/WalkFX.mp3");

//Inputs and Camera Controls
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3(163, 0, 81);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;
int Vista = 0;

float speed = 18.0f; // 3 units / second
float mouseSpeed = 0.005f;
bool isMouseEnabled = true;

void computeMatricesFromInputs() {

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1080 / 2, 720 / 2);
	
	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1080 / 2 - xpos);
	verticalAngle += mouseSpeed * float(720 / 2 - ypos);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
		Vista = 1;
		FXWalk->Playback();
	}    
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
		Vista = -1;
		FXWalk->Playback();
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
		Vista = 1;
		FXWalk->Playback();
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
		Vista -= 1;
		FXWalk->Playback();
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		isMouseEnabled = !isMouseEnabled; // Invierte el valor de la variable isMouseEnabled
		glfwSetInputMode(window, GLFW_CURSOR, isMouseEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}

	bool no_key_pressed = true;
	for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
		if (glfwGetKey(window, key) == GLFW_PRESS) {
			no_key_pressed = false;
			break;
		}
	}

	if (no_key_pressed) {
		FXWalk->Stop();
	}

	FXWalk->Update(0.5f);

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 2000.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

//Inicia la Scene principal
Scene* MainScene;

int main(void)
{
	// Inicializa GLFW
	if (!glfwInit())
	{
		cout << "Error inicializando GLFW" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Crea una ventana GLFW
	window = glfwCreateWindow(1080, 720, "Capybara experience", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetWindowPos(window, 500, 100);
	// Establece el contexto de la ventana como el contexto de OpenGL actual
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glEnable(GL_DEPTH_TEST); // Depth Testing
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Tiempo para FPS
	double currentTime = glfwGetTime();
	//Frames
	int frames = 0;
	//DeltaTime
	double prevTime = glfwGetTime();

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1080 / 2, 720 / 2);

	MainScene = new Scene();
	vec3 AntPos = position;

	// Bucle principal de juego
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window))
	{
		double current = glfwGetTime();
		double deltaT = current - prevTime;
		prevTime = current;

		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
		glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		
		//Renderiza el juego
		MainScene->Render(window, MVP, ViewMatrix, ModelMatrix, ModelView3x3Matrix, ProjectionMatrix, position);
		position.y = MainScene->Get_YPosition();
		if(MainScene->SetNewPosCollider())
		{
			position = AntPos;
		} 
		AntPos = position;
		//cout << "Position: x:" << position.x << " z:" << position.z << "\r";

		//Frames
		frames++;
		double newTime = glfwGetTime();
		if (newTime - currentTime >= 1.0) {
			cout << "FPS: " << (float)frames / (float)(newTime - currentTime) << " Pos: x:" << position.x << " z:" << position.z << "\r";
			frames = 0;
			currentTime = newTime;
		}

		// Intercambia los buffers
		glfwSwapBuffers(window);
		// Verifica eventos
		glfwPollEvents();
	}

	// Termina GLFW
	glfwTerminate();

	//Eliminamos la memoria de la Escena Principal
	delete MainScene;

	return 0;
}