// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <sstream>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

// ATTN 1A is the general place in the program where you have to change the code base to satisfy a Task of Project 1A.
// ATTN 1B for Project 1B. ATTN 1C for Project 1C. Focus on the ones relevant for the assignment you're working on.

typedef struct Vertex {
	float Position[4];
	float Color[4];
	void SetCoords(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = coords[3];
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
};

// ATTN: use POINT structs for cleaner code (POINT is a part of a vertex)
// allows for (1-t)*P_1+t*P_2  avoiding repeat for each coordinate (x,y,z)
typedef struct point {
	float x, y, z;
	point(const float x = 0, const float y = 0, const float z = 0) : x(x), y(y), z(z){};
	point(float *coords) : x(coords[0]), y(coords[1]), z(coords[2]){};
	point operator -(const point& a) const {
		return point(x - a.x, y - a.y, z - a.z);
	}
	point operator +(const point& a) const {
		return point(x + a.x, y + a.y, z + a.z);
	}
	point operator *(const float& a) const {
		return point(x * a, y * a, z * a);
	}
	point operator /(const float& a) const {
		return point(x / a, y / a, z / a);
	}
	float* toArray() {
		float array[] = { x, y, z, 1.0f };
		return array;
	}
};

// Function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], GLushort[], int);
void createObjects(void);
void pickVertex(void);
void moveVertex(void);
void renderScene(void);
void cleanup(void);
static void mouseCallback(GLFWwindow*, int, int, int);

// GLOBAL VARIABLES
GLFWwindow* window;
const GLuint window_width = 1024, window_height = 768;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

// Program IDs
GLuint programID;
GLuint pickingProgramID;

// Uniform IDs
GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint pickingColorID;

GLuint gPickedIndex;
std::string gMessage;

// ATTN: INCREASE THIS NUMBER AS YOU CREATE NEW OBJECTS
const GLuint NumObjects = 1; // Number of objects types in the scene

// Keeps track of IDs associated with each object
GLuint VertexArrayId[NumObjects];
GLuint VertexBufferId[NumObjects];
GLuint IndexBufferId[NumObjects];

size_t VertexBufferSize[NumObjects];
size_t IndexBufferSize[NumObjects];
size_t NumVerts[NumObjects];	// Useful for glDrawArrays command
size_t NumIdcs[NumObjects];	// Useful for glDrawElements command

// Initialize ---  global objects -- not elegant but ok for this project
const size_t IndexCount = 10;
Vertex Vertices[IndexCount];
GLushort Indices[IndexCount];

// ATTN: DON'T FORGET TO INCREASE THE ARRAY SIZE IN THE PICKING VERTEX SHADER WHEN YOU ADD MORE PICKING COLORS
float pickingColor[IndexCount];

bool initFlag = true;


int initWindow(void) {
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // FOR MAC

	// ATTN: Project 1A, Task 0 == Change the name of the window
	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Junnuthula,Mayur Reddy(36921238)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI display
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void) {
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for Project 1, use an ortho camera :
	gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(
		glm::vec3(0, 0, -5), // Camera is at (0,0,-5) below the origin, in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is looking up at the origin (set to 0,-1,0 to look upside-down)
	);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("p1_StandardShading.vertexshader", "p1_StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("p1_Picking.vertexshader", "p1_Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	
	// Get a handle for our "pickingColorID" uniform
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");

	// Define pickingColor array for picking program
	// use a for-loop here
	for (int i = 0; i < IndexCount; i++) {
		pickingColor[i] = i / 255.0f;
	}

	// Define objects
	createObjects();

	// ATTN: create VAOs for each of the newly created objects here:
	// for several objects of the same type use a for-loop
	int obj = 0;  // initially there is only one type of object 
	VertexBufferSize[obj] = sizeof(Vertices);
	IndexBufferSize[obj] = sizeof(Indices);
	NumIdcs[obj] = IndexCount;

	createVAOs(Vertices, Indices, obj);
}

// this actually creates the VAO (structure) and the VBO (vertex data buffer)
void createVAOs(Vertex Vertices[], GLushort Indices[], int ObjectId) {
	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
}

void createObjects(void) {
	// ATTN: DERIVE YOUR NEW OBJECTS HERE:  each object has
	// an array of vertices {pos;color} and
	// an array of indices (no picking needed here) (no need for indices)
	// ATTN: Project 1A, Task 1 == Add the points in your scene

	if (initFlag) {

		Vertices[0].SetCoords(new float[4] {0, 0.0f, 0.0f, 1.0f});
		Vertices[1].SetCoords(new float[4] {0.809f, 0.588f, 0.0f, 1.0f});
		Vertices[2].SetCoords(new float[4] {0.501f, 1.541f, 0.0f, 1.0f});
		Vertices[3].SetCoords(new float[4] {-0.501f, 1.541f, 0.0f, 1.0f});
		Vertices[4].SetCoords(new float[4] {-0.809f, 0.588f, 0.0f, 1.0f});
		Vertices[5].SetCoords(new float[4] {0, 0.0f, 0.0f, 1.0f});
		Vertices[6].SetCoords(new float[4] {0.809f, -0.588f, 0.0f, 1.0f});
		Vertices[7].SetCoords(new float[4] {0.501f, -1.541f, 0.0f, 1.0f});
		Vertices[8].SetCoords(new float[4] {-0.501f, -1.541f, 0.0f, 1.0f});
		Vertices[9].SetCoords(new float[4] {-0.809f, -0.588f, 0.0f, 1.0f});

		for (int i = 0; i < IndexCount; i++) {
			Indices[i] = i;
		}

		/*for (int i = 0; i < IndexCount; i++)
		{
			float theta = 2.0f * 3.1415926f * float(i) / float(IndexCount); 

			float x = 1 * cosf(theta);
			float y = 1 * sinf(theta);
			Vertices[i].SetCoords( new float[4] { x, y, 0.0f, 1.0f });
			Indices[i] = i;
		}*/

		initFlag = false;
	}

	Vertices[0].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[1].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[2].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[3].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[4].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[5].SetColor(new float[4] { 1.0f, 1.0f, 0.0f, 1.0f });
	Vertices[6].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[7].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[8].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[9].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });


	// ATTN: Project 1B, Task 1 == create line segments to connect the control points

	// ATTN: Project 1B, Task 2 == create the vertices associated to the smoother curve generated by subdivision

	// ATTN: Project 1B, Task 4 == create the BB control points and apply De Casteljau's for their corresponding for each piece

	// ATTN: Project 1C, Task 3 == set coordinates of yellow point based on BB curve and perform calculations to find
	// the tangent, normal, and binormal
}


float* prevColor;

void pickVertex(void) {
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // initialization
		// ModelMatrix == TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		// MVP should really be PVM...
		// Send the MVP to the shader (that is currently bound)
		// as data type uniform (shared by all shader instances)
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		
		// pass in the picking color array to the shader
		glUniform1fv(pickingColorArrayID, IndexCount, pickingColor);

		// --- enter vertices into VBO and draw
		glEnable(GL_PROGRAM_POINT_SIZE);
		glBindVertexArray(VertexArrayId[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[0], Vertices);	// update buffer data
		glDrawElements(GL_POINTS, NumIdcs[0], GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
	glFlush();
	// --- Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// --- Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];  // 2x2 pixel region
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
       	// window_height - ypos;  
	// OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	// ATTN: Project 1A, Task 2
	// Find a way to change color of selected vertex and
	// store original color
	prevColor = Vertices[gPickedIndex].Color;

	float newColor[4] = { 0, 0, 0, 1 };
	Vertices[gPickedIndex].SetColor(newColor);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[0], Vertices, GL_STATIC_DRAW);
	
	// Uncomment these lines if you wan to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the visible rendering
}

// ATTN: Project 1A, Task 3 == Retrieve your cursor position, get corresponding world coordinate, and move the point accordingly

// ATTN: Project 1C, Task 1 == Keep track of z coordinate for selected point and adjust its value accordingly based on if certain
// buttons are being pressed

float* worldCoords; //to be used in moveVertex and mouseCallback

void moveVertex(void) {
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);
	double xpos, ypos;

	glfwGetCursorPos(window, &xpos, &ypos);
	vec3 unprojected = glm::unProject(glm::vec3(xpos, ypos, 0.0), ModelMatrix, gProjectionMatrix, vp);
	worldCoords = new float[4]{ -unprojected[0], -unprojected[1], unprojected[2], 1.0f};

	float newColor[4] = { 0, 0, 0, 1 };
	Vertices[gPickedIndex].SetCoords(worldCoords);
	Vertices[gPickedIndex].SetColor(newColor);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[0], Vertices, GL_STATIC_DRAW);

	if (gPickedIndex >= IndexCount) { 
		// Any number > vertices-indices is background!
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	renderScene();
}

void renderScene(void) {    
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Re-clear the screen for visible rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		// see comments in pick
		glm::mat4 ModelMatrix = glm::mat4(1.0); 
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		
		glEnable(GL_PROGRAM_POINT_SIZE);

		glBindVertexArray(VertexArrayId[0]);	// Draw Vertices
		glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[0], Vertices);		// Update buffer data
		glDrawElements(GL_POINTS, NumIdcs[0], GL_UNSIGNED_SHORT, (void*)0);
		// // If don't use indices
		// glDrawArrays(GL_POINTS, 0, NumVerts[0]);	

		// ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE
		// one set per object:
		// glBindVertexArray(VertexArrayId[<x>]); etc etc

		// ATTN: Project 1C, Task 2 == Refer to https://learnopengl.com/Getting-started/Transformations and
		// https://learnopengl.com/Getting-started/Coordinate-Systems - draw all the objects associated with the
		// curve twice in the displayed fashion using the appropriate transformations

		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void cleanup(void) {
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

// Alternative way of triggering functions on mouse click and keyboard events
static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickVertex();
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		Vertices[gPickedIndex].SetColor(prevColor);
		Vertices[gPickedIndex].SetCoords(worldCoords);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[0], Vertices, GL_STATIC_DRAW);
	}
}

int main(void) {
	// ATTN: REFER TO https://learnopengl.com/Getting-started/Creating-a-window
	// AND https://learnopengl.com/Getting-started/Hello-Window to familiarize yourself with the initialization of a window in OpenGL

	// Initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// ATTN: REFER TO https://learnopengl.com/Getting-started/Hello-Triangle to familiarize yourself with the graphics pipeline
	// from setting up your vertex data in vertex shaders to rendering the data on screen (everything that follows)

	// Initialize OpenGL pipeline
	initOpenGL();

	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Timing 
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// DRAGGING: move current (picked) vertex with cursor
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
			moveVertex();
		}

		// ATTN: Project 1B, Task 2 and 4 == account for key presses to activate subdivision and hiding/showing functionality
		// for respective tasks

		// DRAWING the SCENE
		createObjects();	// re-evaluate curves in case vertices have been moved
		renderScene();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}
