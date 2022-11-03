// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
#include <iostream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
using namespace std;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/tiny_obj_loader.h>

const int window_width = 1024, window_height = 768;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], GLushort[], int);
void loadObject(char*, glm::vec4, Vertex* &, GLushort* &, size_t &, int);
void createObjects(void);
void pickObject(void);
void renderScene(float);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
void translateObject(Vertex*& Verts, vec3 trans, int ObjectId, size_t VertCount);
void rotatePen(float);

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 9;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects];
GLuint VertexBufferId[NumObjects];
GLuint IndexBufferId[NumObjects];

// TL
size_t VertexBufferSize[NumObjects];
size_t IndexBufferSize[NumObjects];
size_t NumIdcs[NumObjects];
size_t NumVerts[NumObjects];

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;

glm::vec3 cameraPos, cameraFront, cameraUp, cameraRight, worldUp;
GLfloat cameraYaw, cameraPitch, cameraMoveSpeed, cameraTurnSpeed;

const float toRadians = 3.14159265f / 180.0f;
const float M_PI = 3.14159265f;

// Declare global objects
// TL
const size_t CoordVertsCount = 6;
Vertex CoordVerts[CoordVertsCount];

const size_t GridVertsCount = 12 * 12;
Vertex GridVerts[GridVertsCount];
GLushort GridIndices[GridVertsCount];

size_t BaseVertCount, PenVertCount, TopVertCount, Arm1VertCount, Arm2VertCount, ButtonVertCount, JointVertCount;
Vertex* BaseVerts, *PenVerts, *TopVerts, *Arm1Verts, *Arm2Verts, *ButtonVerts, *JointVerts;

bool bPress = false, pPress = false, shiftPress = false;

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
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// FOR MAC

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

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void cameraUpdate() {
	/*cameraFront.x = cos(radians(cameraYaw)) * cos(radians(cameraPitch));
	cameraFront.y = sin(radians(cameraPitch));
	cameraFront.z = sin(radians(cameraYaw)) * cos(radians(cameraPitch));
	cameraFront = normalize(cameraFront);*/


	cameraRight = normalize(cross(cameraFront, worldUp));
	cameraUp = normalize(cross(cameraRight, cameraFront));

	//gViewMatrix = lookAt(cameraPos, cameraFront, worldUp);
}

void initOpenGL(void) {
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	cameraPos = vec3(10.0f, 10.0f, 10.0f);
	worldUp = vec3(0.0, 1.0, 0.0);
	cameraFront = vec3(0.0, 0.0, 0.0);
	cameraPitch = 90, cameraYaw = 0;
	cameraMoveSpeed = 5, cameraTurnSpeed = 1;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	//gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
	//	glm::vec3(0.0, 0.0, 0.0),	// center
	//	glm::vec3(0.0, 1.0, 0.0));	// up

	std::cout << "initial cameraPos: " << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;
	//cameraUpdate();
	
	gViewMatrix = glm::lookAt(cameraPos,	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		worldUp);	// up


	cameraRight = normalize(cross(cameraFront, worldUp));
	cameraUp = normalize(cross(cameraRight, cameraFront));
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// TL
	// Define objects
	createObjects();

	// ATTN: create VAOs for each of the newly created objects here:
	VertexBufferSize[0] = sizeof(CoordVerts);
	NumVerts[0] = CoordVertsCount;

	VertexBufferSize[1] = sizeof(GridVerts);
	NumVerts[1] = GridVertsCount;

	createVAOs(CoordVerts, NULL, 0);
	createVAOs(GridVerts, NULL, 1);
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {
	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);	// TL

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

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

// Ensure your .obj files are in the correct format and properly loaded by looking at the following function
void loadObject(char* file, glm::vec4 color, Vertex* &out_Vertices, GLushort* &out_Indices, size_t &VertCount, int ObjectId) {
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();
	VertCount = vertCount;

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIdcs[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}

void createObjects(void) {
	//-- COORDINATE AXES --//
	CoordVerts[0] = { { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[1] = { { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[2] = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[3] = { { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[4] = { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
	CoordVerts[5] = { { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } }; 
	
	//-- GRID --//
	// ATTN: Create your grid vertices here!
	int ind = 0;
	float z = -5.0, x = -5.0;
	bool f = true;

	while (z <= 5.0) {
		if (f)
			GridVerts[ind] = { { -5.0, 0.0, z, 1.0 }, { 255.0, 255.0, 255.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		else
			GridVerts[ind] = { { 5.0, 0.0, z, 1.0}, {255.0, 255.0, 255.0, 1.0}, {0.0, 0.0, 1.0} }, z++;

		std::cout << GridVerts[ind].Position[0] << " " << GridVerts[ind].Position[2] << std::endl;
		f = !f;
		ind++;
	}

	f = true;
	while (x <= 5.0) {
		if (f)
			GridVerts[ind] = { { x, 0.0, -5.0, 1.0 }, { 255.0, 255.0, 255.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		else
			GridVerts[ind] = { { x, 0.0, 5.0, 1.0}, {255.0, 255.0, 255.0, 1.0}, {0.0, 0.0, 1.0} }, x++;

		std::cout << GridVerts[ind].Position[0] << " " << GridVerts[ind].Position[2] << std::endl; 
		f = !f;
		ind++;
	}

	for (int i = 0; i < GridVertsCount; i++) {
		GridIndices[i] = i;
	}

	//-- .OBJs --//
	// ATTN: Load your models here through .obj files -- example of how to do so is as shown
	// Vertex* Verts;
	// GLushort* Idcs;
	// loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, ObjectID);
	// createVAOs(Verts, Idcs, ObjectID);

	Vertex* Verts;
	GLushort* Idcs;
	size_t VertCount;
	loadObject("models/Base.obj", glm::vec4(255.0, 0.0, 0.0, 1.0), Verts, Idcs, VertCount, 2);
	createVAOs(Verts, Idcs, 2);
	BaseVerts = Verts;
	BaseVertCount = VertCount;

	loadObject("models/Top.obj", glm::vec4(255.0, 165.0, 0.0, 1.0), Verts, Idcs, VertCount, 3);
	createVAOs(Verts, Idcs, 3);
	TopVerts = Verts;
	TopVertCount = VertCount;

	loadObject("models/Arm1.obj", glm::vec4(0.0, 0.0, 255.0, 1.0), Verts, Idcs, VertCount, 4);
	createVAOs(Verts, Idcs, 4);
	Arm1Verts = Verts;
	Arm1VertCount = VertCount;

	loadObject("models/Joint.obj", glm::vec4(128.0, 0.0, 128.0, 1.0), Verts, Idcs, VertCount, 5);
	createVAOs(Verts, Idcs, 5);
	JointVerts = Verts;
	JointVertCount = VertCount;

	loadObject("models/Arm2.obj", glm::vec4(173.0, 216.0, 230.0, 1.0), Verts, Idcs, VertCount, 6);
	createVAOs(Verts, Idcs, 6);
	Arm2Verts = Verts;
	Arm2VertCount = VertCount;

	loadObject("models/Pen.obj", glm::vec4(255.0, 255.0, 0.0, 1.0), Verts, Idcs, VertCount, 7);
	createVAOs(Verts, Idcs, 7);
	PenVerts = Verts;
	PenVertCount = VertCount;

	loadObject("models/Button.obj", glm::vec4(255.0, 0.0, 0.0, 1.0), Verts, Idcs, VertCount, 8);
	createVAOs(Verts, Idcs, 8);
	ButtonVerts = Verts;
	ButtonVertCount = VertCount;
}

void pickObject(void) {
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	if (gPickedIndex == 255) { // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void renderScene(float deltaTime) {
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[0]);	// Draw CoordAxes
		glDrawArrays(GL_LINES, 0, NumVerts[0]);

		glBindVertexArray(VertexArrayId[1]);	// Draw CoordAxes
		//glDrawArrays(GL_LINES, 0, NumVerts[1]);
		glDrawArrays(GL_LINES, 0, NumVerts[1]);

		//glBindVertexArray(VertexArrayId[2]);	// Draw CoordAxes
		////glDrawArrays(GL_LINES, 0, NumVerts[2]);
		//glDrawArrays(GL_TRIANGLES, 0, NumVerts[2]);

		glBindVertexArray(VertexArrayId[2]);	// Draw Vertices
		glDrawElements(GL_TRIANGLES, NumIdcs[2], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(VertexArrayId[3]);	// Draw Vertices
		glDrawElements(GL_TRIANGLES, NumIdcs[3], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(VertexArrayId[4]);	// Draw Vertices
		glDrawElements(GL_TRIANGLES, NumIdcs[4], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(VertexArrayId[5]);	// Draw Vertices
		glDrawElements(GL_TRIANGLES, NumIdcs[5], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(VertexArrayId[6]);	// Draw Vertices
		glDrawElements(GL_TRIANGLES, NumIdcs[6], GL_UNSIGNED_SHORT, (void*)0);

		if (pPress) {
			rotatePen(deltaTime);
		}

		glBindVertexArray(VertexArrayId[7]);	// Draw Vertices
		glDrawElements(GL_TRIANGLES, NumIdcs[7], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(VertexArrayId[8]);	// Draw Vertices
		glDrawElements(GL_TRIANGLES, NumIdcs[8], GL_UNSIGNED_SHORT, (void*)0);
			
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

//void moveCamera() {
//	glm::vec4 position(cameraPos.x, cameraPos.y, cameraPos.z, 1);
//	glm::vec4 pivot(cameraFront.x, cameraFront.y, cameraFront.z, 1);
//
//	float deltaAngleX = (2 * M_PI / window_width); // a movement from left to right = 2*PI = 360 deg
//	float deltaAngleY = (M_PI / window_height);  // a movement from top to bottom = PI = 180 deg
//	float xAngle = 0;
//	float yAngle = 0;
//
//	if (glfwGetKey(window, GLFW_KEY_LEFT))
//		xAngle = (1) * deltaAngleX;
//
//	else if (glfwGetKey(window, GLFW_KEY_LEFT))
//		xAngle = (-1) * deltaAngleX;
//
//	else if (glfwGetKey(window, GLFW_KEY_UP))
//		yAngle = (1) * deltaAngleY;
//
//	else if (glfwGetKey(window, GLFW_KEY_DOWN))
//		yAngle = (-1) * deltaAngleY;
//
//	else return;
//
//	//float yAngle = (1) * deltaAngleY;
//	//float xAngle = (1) * deltaAngleX;
//	//float yAngle = (1) * deltaAngleY;
//
//	float cosAngle = dot(cameraFront, cameraUp);
//	if (cosAngle * sign(deltaAngleY) > 0.99f)
//		deltaAngleY = 0;
//
//	glm::mat4x4 rotationMatrixX(1.0f);
//	rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, cameraUp);
//	position = (rotationMatrixX * (position - pivot)) + pivot;
//
//	glm::mat4x4 rotationMatrixY(1.0f);
//	rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, cameraRight);
//	glm::vec4 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;
//
//	cameraPos = vec3(finalPosition.x, finalPosition.y, finalPosition.z);
//	
//	gViewMatrix = glm::lookAt(cameraPos, cameraFront, cameraUp);
//
//	std::cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;
//}

void translateObject(Vertex* &Verts, vec3 trans, int ObjectId, size_t VertCount) {
	trans *= 0.05;
	for (int i = 0; i < VertCount; i++) {
		vec4 vec(Verts[i].Position[0] + trans[0], Verts[i].Position[1] + trans[1], Verts[i].Position[2] + trans[2], Verts[i].Position[3]);
		//std::cout <<"moved verts: " << vec.x << " " << vec.y << " " << vec.z << std::endl;
		Verts[i].SetPosition(new float[4] {vec[0], vec[1], vec[2], vec[3]});
	}

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Verts, GL_STATIC_DRAW);
}

vec3 avgPos(Vertex* Verts, const size_t VertCount) {
	float sx = 0, sy = 0, sz = 0;

	for (int i = 0; i < VertCount; i++) {
		sx += Verts[i].Position[0];
		sy += Verts[i].Position[1];
		sz += Verts[i].Position[2];
	}
	
	return vec3(sx / VertCount, sy / VertCount, sz / VertCount);
}

vec3 avgNorm(Vertex* Verts, const size_t VertCount) {
	float sx = 0, sy = 0, sz = 0;

	for (int i = 0; i < VertCount; i++) {
		sx += Verts[i].Normal[0];
		sy += Verts[i].Normal[1];
		sz += Verts[i].Normal[2];
	}

	return vec3(sx / VertCount, sy / VertCount, sz / VertCount);
}

float penTheta = 0;
void rotatePen(float deltaTime) {
	if (!pPress) return;

	int lastInd = Arm2VertCount - 1, lastPind = PenVertCount - 1, pmid = lastPind / 2;
	//vec3 axis = vec3(Arm2Verts[lastInd].Position[0] - PenVerts[lastPind].Position[0], Arm2Verts[lastInd].Position[1] - PenVerts[lastPind].Position[1], Arm2Verts[lastInd].Position[2] - PenVerts[lastPind].Position[2]);
	vec3 trans = avgPos(PenVerts, PenVertCount);
	vec3 axis = avgNorm(PenVerts, PenVertCount);

	//axis = cross(axis, vec3(PenVerts[0].Position[0], PenVerts[0].Position[1], PenVerts[0].Position[2]));

	glm:mat4 model = mat4(1.f);

	/*glTranslatef(-trans.x, -trans.y, -trans.z);
	glRotatef((float)glfwGetTime(), trans.x, trans.y, trans.z);
	glTranslatef(trans.x, trans.y, trans.z);*/

	if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		cout << "shift pressed" << endl;
		bool f = false;
		if (glfwGetKey(window, GLFW_KEY_LEFT)) {
			penTheta -= 1.f;
			f = true;
			cout << "left pressed" << endl;
		}

		else if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
			penTheta += 1.f;
			f = true;
			cout << "right pressed" << endl;
		}

		if (f) {
			model = glm::translate(model, vec3(-trans.x, -trans.y, -trans.z));
			model = glm::rotate(model, penTheta * 0.01f, normalize(trans));
			model = glm::translate(model, vec3(trans.x, trans.y, trans.z));

			////model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
			////model = glm::scale(model, glm::vec3(10, 10, 10));
		}

		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &model[0][0]);
	}

	//cout << PenVerts[0].Position[0] << " " << PenVerts[0].Position[1] << " " << PenVerts[0].Position[2] << endl;
}

void moveBase() {
	if (!bPress) return;

	vec3 trans;
	
	if (glfwGetKey(window, GLFW_KEY_LEFT)) {
		//std::cout << "KeyPressed" << std::endl;
		trans = vec3(-1.0f, 0.0f, 0.0f);
	}

	else if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
		//std::cout << "KeyPressed" << std::endl;
		trans = vec3(1.0f, 0.0f, 0.0f);
	}

	if (glfwGetKey(window, GLFW_KEY_UP)) {
		//std::cout << "KeyPressed" << std::endl;
		trans = vec3(0.0f, 0.0f, -1.0f);
	}

	else if (glfwGetKey(window, GLFW_KEY_DOWN)) {
		//std::cout << "KeyPressed" << std::endl;
		trans = vec3(0.0f, 0.0f, 1.0f);
	}

	//std::cout << vec.x << " " << vec.y <<" " <<  vec.z << std::endl;
	translateObject(BaseVerts, trans, 2, BaseVertCount);
	translateObject(TopVerts, trans, 3, TopVertCount);
	translateObject(Arm1Verts, trans, 4, Arm1VertCount);
	translateObject(JointVerts, trans, 5, JointVertCount);
	translateObject(Arm2Verts, trans, 6, Arm2VertCount);
	translateObject(PenVerts, trans, 7, PenVertCount);
	translateObject(ButtonVerts, trans, 8, ButtonVertCount);
}

float prevX = 10.f, prevZ = 10.f, prevY = 10.f;
// Alternative way of triggering functions on keyboard events
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_P:
			pPress = !pPress;
			break;
		case GLFW_KEY_B:
			bPress = !bPress;
			break;
		case GLFW_KEY_A:
			break;
		case GLFW_KEY_D:
			break;
		case GLFW_KEY_W:
			break;
		case GLFW_KEY_S:
			break;
		case GLFW_KEY_SPACE:
			break;
		/*case GLFW_KEY_RIGHT_SHIFT:
			if (pPress) shiftPress = !shiftPress;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			if (pPress) shiftPress = !shiftPress;
			break;*/
		default:
			break;
		}
	}
}

// Alternative way of triggering functions on mouse click events
static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}


int main(void) {
	// TL
	// ATTN: Refer to https://learnopengl.com/Getting-started/Transformations, https://learnopengl.com/Getting-started/Coordinate-Systems,
	// and https://learnopengl.com/Getting-started/Camera to familiarize yourself with implementing the camera movement

	// ATTN (Project 3 only): Refer to https://learnopengl.com/Getting-started/Textures to familiarize yourself with mapping a texture
	// to a given mesh

	// Initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// Initialize OpenGL pipeline
	initOpenGL();

	double lastTime = 0;
	float theta = asin(10 / sqrt(200)), phi = atan(1);
	const float radius = 10.0f;
	// For speed computation
	//double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		double deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		if (glfwGetKey(window, GLFW_KEY_A)) {
			/*float cenX = prevX - radius * sin(5.f);
			float cenZ = prevZ - radius * cos(5.f)*/;
			theta -= radians(0.1f);
			prevX = 0.0 + sin(theta) * cos(phi) * radius;
			prevZ = 0.0 + cos(theta) * cos(phi) * radius;
			//glm::mat4 view;
			gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		}

		if (glfwGetKey(window, GLFW_KEY_D)) {
			/*float cenX = prevX - radius * sin(5.f);
			float cenZ = prevZ - radius * cos(5.f)*/;
			theta += radians(0.1f);
			prevX = 0.0 + sin(theta) * cos(phi) * radius;
			prevZ = 0.0 + cos(theta) * cos(phi) * radius;
			//glm::mat4 view;
			gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		}

		if (glfwGetKey(window, GLFW_KEY_W)) {
			/*float cenX = prevX - radius * sin(5.f);
			float cenZ = prevZ - radius * cos(5.f)*/;
			phi += radians(0.1f);
			prevY = 0.0 + sin(phi) * radius;
			prevZ = 0.0 + cos(theta) * cos(phi) * radius;
			//glm::mat4 view;
			gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		}

		if (glfwGetKey(window, GLFW_KEY_S)) {
			/*float cenX = prevX - radius * sin(5.f);
			float cenZ = prevZ - radius * cos(5.f)*/;
			phi -= radians(0.1f);
			prevY = 0.0 + sin(phi) * radius;
			prevZ = 0.0 + cos(theta) * cos(phi) * radius;
			//glm::mat4 view;
			gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		}


		//if (glfwGetKey(window, GLFW_KEY_A)) {
		//	/*float dx = prevX - 0.001f, dz = prevZ - 0.001f;
		//	prevX = dx, prevZ = dz;*/
		//	glRotatef(radians(5.f), 0, 0, 0);
		//	//prevX += sqrt(200) * sinf(5) * deltaTime, prevZ += sqrt(200) * cosf(5) * deltaTime;
		//	////gViewMatrix = glm::lookAt(glm::vec3(sinf(prevX) * 0.001, prevY, cosf(prevZ) * 0.001), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		//	//gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		//	//cout << prevX << " " << prevY << " " << prevZ << endl;
		//	/*gViewMatrix = glm::rotate(gViewMatrix, radians(5.f), vec3(0.f, 1.f, 0.f));
		//	vec4 pos = vec4(cameraPos[0], cameraPos[1], cameraPos[2], 1.f);
		//	pos *= gViewMatrix * (float)deltaTime;
		//	cameraPos = vec3(pos[0], pos[1], pos[2]);
		//	cout << pos[0] << " " << pos[1] << " " << pos[2] << endl;
		//	gViewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));*/

		//}

		//if (glfwGetKey(window, GLFW_KEY_D)) {
		///*	float dx = prevX + 0.001f, dz = prevZ + 0.001f;
		//	prevX = dx, prevZ = dz;
		//	gViewMatrix = glm::lookAt(glm::vec3(sinf(prevX) * 0.001, prevY, cosf(prevZ) * 0.001), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));*/
		//	prevX -= sqrt(200) * sinf(5) * deltaTime, prevZ -= sqrt(200) * cosf(5) * deltaTime;
		//	gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		//	cout << prevX << " " << prevY << " " << prevZ << endl;
		//	/*gViewMatrix = glm::rotate(gViewMatrix, radians(-5.f), vec3(0.f, 1.f, 0.f));
		//	vec4 pos = vec4(cameraPos[0], cameraPos[1], cameraPos[2], 1.f);
		//	pos = (float)deltaTime * gViewMatrix * pos;
		//	cameraPos = vec3(pos[0], pos[1], pos[2]);
		//	cout << pos[0] << " " << pos[1] << " " << pos[2] << endl;
		//	gViewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));*/
		//}

		/*if (glfwGetKey(window, GLFW_KEY_W)) {
			float dy = prevY + 0.001f, dz = prevZ + 0.001f;
			prevY = dy, prevZ = dz;
			gViewMatrix = glm::lookAt(glm::vec3(prevX, sinf(prevY) * 0.01, cosf(prevZ) * 0.001), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		}

		if (glfwGetKey(window, GLFW_KEY_S)) {
			float dy = prevY + 0.001f, dz = prevZ + 0.001f;
			prevY = dy, prevZ = dz;
			gViewMatrix = glm::lookAt(glm::vec3(prevX, sinf(prevY) * 0.01, cosf(prevZ) * 0.001), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		}*/

		//if (glfwGetKey(window, GLFW_KEY_UP)) {
		//	cameraPos.y += r * cosf(radians(10.f)) * vel, cameraPos.z += r * sinf(radians(10.f)) * vel;
		//	cameraUpdate();
		//	cameraPos = rotate()
		//	gViewMatrix = lookAt(cameraPos, vec3(0, 0, 0), cameraUp);
		//	std::cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;
		//	//cameraUpdate();
		//}

		//if (glfwGetKey(window, GLFW_KEY_DOWN)) {
		//	/*cameraPos -= cameraFront * vel;
		//	cameraUpdate();*/
		//	cameraPos.y -= r * cosf(radians(10.f)) * vel, cameraPos.z -= r * sinf(radians(10.f)) * vel;
		//	cameraUpdate();
		//	gViewMatrix = lookAt(cameraPos, vec3(0, 0, 0), cameraUp);
		//	std::cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;
		//	gViewMatrix = lookAt(cameraPos, vec3(0, 0, 0), cameraUp);
		//	std::cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;

		//	//cameraUpdate();
		//}

		//moveCamera();
		/*if (glfwGetKey(window, GLFW_KEY_LEFT)) {
			cameraPos -= cameraRight * cameraMoveSpeed * cos(radians(cameraYaw));
			cameraUpdate();
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
			cameraPos += cameraRight * cameraMoveSpeed * cos(radians(cameraYaw));
			cameraUpdate();
		}*/

		if (bPress) {
			moveBase();
		}

		// DRAWING POINTS
		renderScene(deltaTime);

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}

//float sens = 1.0f;
//bool isCPress = false, isRightPress = false, isLeftPress = false;
//
//static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//	/*if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
//		isCPress = !isCPress;*/
//	const float cameraSpeed = 0.5f; // adjust accordingly
//	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//		cameraPos += cameraSpeed * cameraFront;
//	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//		cameraPos -= cameraSpeed * cameraFront;
//	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
//	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
//}
//
//float camX  = 10, camZ = 10;
//
//void cameraMove() {
//	gViewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
//	/*std::cout << "CPressed: " << isCPress << std::endl;
//	if (!isCPress) return;
//
//	const float radius = 10.0f * sens;
//	
//	camX = sin(glfwGetTime()) * radius;
//	camZ = cos(glfwGetTime()) * radius;
//	*/
//	//gViewMatrix = glm::lookAt(glm::vec3(0.0f, camX, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
//}