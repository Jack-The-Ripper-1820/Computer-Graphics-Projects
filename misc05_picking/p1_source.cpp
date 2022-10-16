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
using namespace std;

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
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void bindBuffers();

// GLOBAL VARIABLES
GLFWwindow* window;
const GLuint window_width = 1024, window_height = 768;
const GLuint origWindowWidth = 1024, origWindowHeight = 768;

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
const GLuint NumObjects = 12; // Number of objects types in the scene

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
Vertex InitVertices[IndexCount];

const size_t SubIndexCount = 620;
Vertex SubVertices[SubIndexCount];
GLushort SubIndices[SubIndexCount];

const size_t BBIndexCount = 40;
Vertex BBVertices[BBIndexCount];
GLushort BBIndices[BBIndexCount];

const size_t CRIndexCount = 40;
Vertex CRVertices[CRIndexCount];
GLushort CRIndices[CRIndexCount];

const size_t CurveIndexCount = 101*3*40 + 1;
Vertex CurveVertices[CurveIndexCount];
GLushort CurveIndices[CurveIndexCount];

Vertex ZYVertices[IndexCount];
Vertex ZYCurveVertices[CurveIndexCount];
Vertex ZYCRVertices[CRIndexCount];

const size_t TriangleIndexCount = 3;
Vertex TriangleVertices[TriangleIndexCount];
GLushort TriangleIndices[TriangleIndexCount];

Vertex ZYTriangleVertices[TriangleIndexCount];

const size_t TNBIndexCount = 6;
Vertex TNBVertices[TNBIndexCount];
GLushort  TNBIndices[TNBIndexCount];

Vertex ZYTNBVertices[TNBIndexCount];

Vertex TangentVertices[CurveIndexCount], NormalVertices[CurveIndexCount], BiNormalVertices[CurveIndexCount];
Vertex ZYTangentVertices[CurveIndexCount], ZYNormalVertices[CurveIndexCount], ZYBiNormalVertices[CurveIndexCount];

// ATTN: DON'T FORGET TO INCREASE THE ARRAY SIZE IN THE PICKING VERTEX SHADER WHEN YOU ADD MORE PICKING COLORS
float pickingColor[IndexCount];

bool initFlag = true;
std::vector<std::vector<Vertex>> P;
bool key1Flag = false;
bool key2Flag = false;
bool key3Flag = false;
bool shiftFlag = false;
bool key4Flag = false;
bool key5Flag = false;
float T = 0.01;

//float prevY[IndexCount];

//private void resizeWindow(int argWidth, int argHeight) {
//	glViewport(0, 0, argWidth, argHeight);
//
//	adjustProjectionMatrix(width, height); // recalculating projection matrix (only if you are using one)
//}

//void window_size_callback(GLFWwindow* window, int width, int height)
//{
//	window_width = width, window_height = height;
//	float ratio = (width * height) / (origWindowWidth * origWindowHeight);
//
//	gProjectionMatrix = glm::ortho(-4.0f * ratio, 4.0f * ratio, -3.0f * ratio, 3.0f * ratio, 0.0f * ratio, 100.0f * ratio); // In world coordinates
//}


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
	glfwSetKeyCallback(window, key_callback);
	//glfwSetWindowSizeCallback(window, window_size_callback);

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
	//gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
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

	obj++;

	VertexBufferSize[obj] = sizeof(SubVertices);
	IndexBufferSize[obj] = sizeof(SubIndices);
	NumIdcs[obj] = SubIndexCount;
	createVAOs(SubVertices, SubIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(BBVertices);
	IndexBufferSize[obj] = sizeof(BBIndices);
	NumIdcs[obj] = BBIndexCount;
	createVAOs(BBVertices, BBIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(CRVertices);
	IndexBufferSize[obj] = sizeof(CRIndices);
	NumIdcs[obj] = CRIndexCount;
	createVAOs(CRVertices, CRIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(CurveVertices);
	IndexBufferSize[obj] = sizeof(CurveIndices);
	NumIdcs[obj] = CurveIndexCount;
	createVAOs(CurveVertices, CurveIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(ZYVertices);
	IndexBufferSize[obj] = sizeof(Indices);
	NumIdcs[obj] = IndexCount;
	createVAOs(ZYVertices, Indices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(ZYCRVertices);
	IndexBufferSize[obj] = sizeof(CRIndices);
	NumIdcs[obj] = CRIndexCount;
	createVAOs(ZYCRVertices, CRIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(ZYCurveVertices);
	IndexBufferSize[obj] = sizeof(CurveIndices);
	NumIdcs[obj] = CurveIndexCount;
	createVAOs(ZYCurveVertices, CurveIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(TriangleVertices);
	IndexBufferSize[obj] = sizeof(TriangleIndices);
	NumIdcs[obj] = TriangleIndexCount;
	createVAOs(TriangleVertices, TriangleIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(ZYTriangleVertices);
	IndexBufferSize[obj] = sizeof(TriangleIndices);
	NumIdcs[obj] = TriangleIndexCount;
	createVAOs(ZYTriangleVertices, TriangleIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(TNBVertices);
	IndexBufferSize[obj] = sizeof(TNBIndices);
	NumIdcs[obj] = TNBIndexCount;
	createVAOs(TNBVertices, TNBIndices, obj);

	obj++;

	VertexBufferSize[obj] = sizeof(ZYTNBVertices);
	IndexBufferSize[obj] = sizeof(TNBIndices);
	NumIdcs[obj] = TNBIndexCount;
	createVAOs(ZYTNBVertices, TNBIndices, obj);
	
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

void bindBuffers() {
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[0], Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[1], SubVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[2], BBVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[3], CRVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[4], CurveVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[5], ZYVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[6], ZYCRVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[7], ZYCurveVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[8], TriangleVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[9]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[9], ZYTriangleVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[10]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[10], TNBVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[11]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[11], ZYTNBVertices, GL_STATIC_DRAW);
}

int k = 0;
int ind = 0;

void subDivide() {
	k++;
	//cout << "k after increment: " << k << endl;
	if (k > 5) {
		k = 0;
		Vertex* v = new Vertex();
		fill(begin(SubVertices), end(SubVertices), *v);
		ind = 0;
		//cout << "in k > 5" << endl;
		key1Flag = false;
		return;
	}

	int n = P[k].size();

	//cout << "k and n : " << k << " " << n << endl;
}

void createBB() {
	//cout << "in CreateBB" << endl;
	vector<vector<vector<float>> > C(10, vector<vector<float>>(4));
	int bbInd = 0;

	for (int i = 0; i < IndexCount; i++) {
		C[i][1] = { (2 * Vertices[i].Position[0] + Vertices[(i + 1) % IndexCount].Position[0]) / 3, (2 * Vertices[i].Position[1] + Vertices[(i + 1) % IndexCount].Position[1]) / 3, 0.0f, 1.0f };
		C[i][2] = { (Vertices[i].Position[0] + 2 * Vertices[(i + 1) % IndexCount].Position[0]) / 3, (Vertices[i].Position[1] + 2 * Vertices[(i + 1) % IndexCount].Position[1]) / 3, 0.0f, 1.0f };
	}

	for (int i = 0; i < 10; i++) {
		C[i][0] = { 0.5f * (Vertices[(i - 1 + IndexCount) % IndexCount].Position[0] + Vertices[(i + 1) % IndexCount].Position[0]) / 3 + (2 * Vertices[i].Position[0] / 3),
			0.5f * (Vertices[(i - 1 + IndexCount) % IndexCount].Position[1] + Vertices[(i + 1) % IndexCount].Position[1]) / 3 + (2 * Vertices[i].Position[1] / 3), 0.0f, 1.0f };
	}

	for (int i = 0; i < 10; i++) {
		C[i][3] = { C[(i + 1) % IndexCount][0][0], C[(i + 1) % IndexCount][0][1], 0.0f, 1.0f };
	}

	if (key2Flag) {
		for (int i = 0; i < 10; i++) {
			Vertex* v1 = new Vertex(), * v2 = new Vertex(), * v3 = new Vertex(), * v4 = new Vertex();
			v1->SetCoords(new float[4] {C[i][0][0], C[i][0][1], 0.0f, 1.0f}), v1->SetColor(new float[4] { 255.0f, 255.0f, 0.0f, 1.0f });
			v2->SetCoords(new float[4] {C[i][1][0], C[i][1][1], 0.0f, 1.0f}), v2->SetColor(new float[4] { 255.0f, 255.0f, 0.0f, 1.0f });
			v3->SetCoords(new float[4] {C[i][2][0], C[i][2][1], 0.0f, 1.0f}), v3->SetColor(new float[4] { 255.0f, 255.0f, 0.0f, 1.0f });
			v4->SetCoords(new float[4] {C[i][3][0], C[i][3][1], 0.0f, 1.0f}), v4->SetColor(new float[4] { 255.0f, 255.0f, 0.0f, 1.0f });
			BBVertices[bbInd++] = *v1, BBVertices[bbInd++] = *v2, BBVertices[bbInd++] = *v3, BBVertices[bbInd++] = *v4;
		}
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[2], BBVertices, GL_STATIC_DRAW);
	}
}

float* getTangentPoint(vector<float> &P0, vector<float>& P1, vector<float>& P2, vector<float>& P3, float t) {
	//P'(t) = -3(1 - t) ^ 2 * P0 + 3(1 - t) ^ 2 * P1 - 6t(1 - t) * P1 - 3t ^ 2 * P2 + 6t(1 - t) * P2 + 3t ^ 2 * P3
	float tangent[2] = { 0.0f, 0.0f };

	tangent[0] = -3 * pow(1 - t, 2) * P0[0] + 3 * pow(1 - t, 2) * P1[0] - 6 * t * (1 - t) * P1[0] - pow(3 * t, 2) * P2[0] + 6 * t * (1 - t) * P2[0] + 3 * pow(3 * t, 2) * P3[0];
	tangent[1] = -3 * pow(1 - t, 2) * P0[1] + 3 * pow(1 - t, 2) * P1[1] - 6 * t * (1 - t) * P1[1] - pow(3 * t, 2) * P2[1] + 6 * t * (1 - t) * P2[1] + 3 * pow(3 * t, 2) * P3[1];

	return tangent;
}

float* bezierTangent(float t, vector<float>& a, vector<float>& b, vector<float>& c, vector<float>& d)
{
	// note that abcd are aka x0 x1 x2 x3

/*  the four coefficients ..
	A = x3 - 3 * x2 + 3 * x1 - x0
	B = 3 * x2 - 6 * x1 + 3 * x0
	C = 3 * x1 - 3 * x0
	D = x0

	and then...
	Vx = 3At2 + 2Bt + C         */

	// first calcuate what are usually know as the coeffients,
	// they are trivial based on the four control points:

	float C1x = (d[0] - (3.0 * c[0]) + (3.0 * b[0]) - a[0]);
	float C2x = ((3.0 * c[0]) - (6.0 * b[0]) + (3.0 * a[0]));
	float C3x = ((3.0 * b[0]) - (3.0 * a[0]));
	float C4x = (a[0]);  // (not needed for this calculation)

	float C1y = (d[1] - (3.0 * c[1]) + (3.0 * b[1]) - a[1]);
	float C2y = ((3.0 * c[1]) - (6.0 * b[1]) + (3.0 * a[1]));
	float C3y = ((3.0 * b[1]) - (3.0 * a[1]));
	float C4y = (a[1]);  // (not needed for this calculation)

	// finally it is easy to calculate the slope element,
	// using those coefficients:

	return new float[4]  {(3.0f * C1x * t * t) + (2.0f * C2x * t) + C3x, (3.0f * C1y * t * t) + (2.0f * C2y * t) + C3y };

	// note that this routine works for both the x and y side;
	// simply run this routine twice, once for x once for y
	// note that there are sometimes said to be 8 (not 4) coefficients,
	// these are simply the four for x and four for y,
	// calculated as above in each case.
}

void createCR() {
	//cout << "in CreateCR" << endl;
	vector<vector<vector<float>> > C(IndexCount, vector<vector<float>>(4));
	int crInd = 0;
	for (int i = 0; i < IndexCount; i++) {
		C[i][0] = { Vertices[i].Position[0], Vertices[i].Position[1], 0.0f, 1.0f };
	}

	for (int i = 0; i < IndexCount; i++) {
		C[i][3] = { Vertices[(i + 1) % IndexCount].Position[0], Vertices[(i + 1) % IndexCount].Position[1], 0.0f, 1.0f };
	}

	for (int i = 0; i < IndexCount; i++) {
		C[i][1] = { Vertices[i].Position[0] + 0.625f * (Vertices[(i + 1) % IndexCount].Position[0] - Vertices[(i - 1 + IndexCount) % IndexCount].Position[0]) / 3, Vertices[i].Position[1] + 0.625f * (Vertices[(i + 1) % IndexCount].Position[1] - Vertices[(i - 1 + IndexCount) % IndexCount].Position[1]) / 3, 0.0f, 1.0f};
		C[(i - 1 + IndexCount) % IndexCount][2] = { Vertices[i].Position[0] - 0.625f * (Vertices[(i + 1) % IndexCount].Position[0] - Vertices[(i - 1 + IndexCount) % IndexCount].Position[0]) / 3, Vertices[i].Position[1] - 0.625f * (Vertices[(i + 1) % IndexCount].Position[1] - Vertices[(i - 1 + IndexCount) % IndexCount].Position[1]) / 3, 0.0f, 1.0f };
	}

	if (key3Flag) {
		for (int i = 0; i < IndexCount; i++) {
			Vertex* v1 = new Vertex(), * v2 = new Vertex(), * v3 = new Vertex(), * v4 = new Vertex();
			v1->SetCoords(new float[4] {C[i][0][0], C[i][0][1], 0.0f, 1.0f}), v1->SetColor(new float[4] { 255.0f, 0.0f, 0.0f, 1.0f });
			v2->SetCoords(new float[4] {C[i][1][0], C[i][1][1], 0.0f, 1.0f}), v2->SetColor(new float[4] { 255.0f, 0.0f, 0.0f, 1.0f });
			v3->SetCoords(new float[4] {C[i][2][0], C[i][2][1], 0.0f, 1.0f}), v3->SetColor(new float[4] { 255.0f, 0.0f, 0.0f, 1.0f });
			v4->SetCoords(new float[4] {C[i][3][0], C[i][3][1], 0.0f, 1.0f}), v4->SetColor(new float[4] { 255.0f, 0.0f, 0.0f, 1.0f });
			CRVertices[crInd++] = *v1, CRVertices[crInd++] = *v2, CRVertices[crInd++] = *v3, CRVertices[crInd++] = *v4;
		}

		vector<vector<float>> Points;
		for (auto& a : C) {
			for (auto& b : a) {
				Points.push_back(b);
			}
		}

		int n = Points.size(), cInd = 0;

		for (int pi = 0; pi < IndexCount; pi++) {
			vector<vector<float>> p = { C[pi][0], C[pi][1], C[pi][2], C[pi][3]}, q = p;
			//0.05882352941f
			for (float t = T; t <= 1.0f; t += T) {
				for (int k = 1; k < 4; k++) {
					for (int i = 0; i < 4 - k; i++) {
						q[i][0] = (1 - t) * q[i][0] + t * q[i + 1][0];
						q[i][1] = (1 - t) * q[i][1] + t * q[i + 1][1];
					}
				}

				//cout << "Curve point: " << q[0][0] << " " << q[0][1] << endl;
				CurveVertices[cInd].SetCoords(new float[4] { q[0][0], q[0][1], 0.0f, 1.0f });
				CurveVertices[cInd].SetColor(new float[4] { 0.0f, 255.0f, 0.0f, 1.0f });

				//cout << "T: " << t << " Ps: " << p[0][0] << ", " << p[0][1] << endl;
				//float* tangentPoint = getTangentPoint(p[0], p[1], p[2], p[3], t);
				float* tangentPoint = bezierTangent(t, p[0], p[1], p[2], p[3]);

				float tx = tangentPoint[0], ty = tangentPoint[1];
				float utx = tx / pow(tx * tx + ty * ty, 0.5), uty = ty / pow(tx * tx + ty * ty, 0.5);
				TangentVertices[cInd].SetCoords(new float[4] {tx + q[0][0], ty + q[0][1], 0.0f, 1.0f});

				float nx = tangentPoint[0] + 1.f * cosf(1.5708), ny = tangentPoint[1] + 1.f * sinf(1.5708);
				float unx = nx / pow(nx * nx + ny * ny, 0.5), uny = ny / pow(nx * nx + ny * ny, 0.5);
				NormalVertices[cInd].SetCoords(new float[4] {nx + q[0][0], ny + q[0][1], 0.0f, 1.0f});

				float bix = tangentPoint[0] + 1.f * cosf(2 * 1.5708), biy = tangentPoint[1] + 1.f * sinf(2 * 1.5708);
				float ubix = bix / pow(bix * bix + biy * biy, 0.5), ubiy = biy / pow(bix * bix + biy * biy, 0.5);
				BiNormalVertices[cInd++].SetCoords(new float[4] {bix + q[0][0], biy + q[0][1], 0.0f, 1.0f});
			}
		}
	}
}

void createZYCR() {
	//cout << "in CreateCR" << endl;
	vector<vector<vector<float>> > C(IndexCount, vector<vector<float>>(4));
	int crInd = 0;
	for (int i = 0; i < IndexCount; i++) {
		C[i][0] = { ZYVertices[i].Position[0], ZYVertices[i].Position[1], 0.0f, 1.0f };
	}

	for (int i = 0; i < IndexCount; i++) {
		C[i][3] = { ZYVertices[(i + 1) % IndexCount].Position[0], ZYVertices[(i + 1) % IndexCount].Position[1], 0.0f, 1.0f };
	}

	for (int i = 0; i < IndexCount; i++) {
		C[i][1] = { ZYVertices[i].Position[0] + 0.625f * (ZYVertices[(i + 1) % IndexCount].Position[0] - ZYVertices[(i - 1 + IndexCount) % IndexCount].Position[0]) / 3, 
			ZYVertices[i].Position[1] + 0.625f * (ZYVertices[(i + 1) % IndexCount].Position[1] - ZYVertices[(i - 1 + IndexCount) % IndexCount].Position[1]) / 3, 0.0f, 1.0f };
		C[(i - 1 + IndexCount) % IndexCount][2] = { ZYVertices[i].Position[0] - 0.625f * (ZYVertices[(i + 1) % IndexCount].Position[0] - ZYVertices[(i - 1 + IndexCount) % IndexCount].Position[0]) / 3,
			ZYVertices[i].Position[1] - 0.625f * (ZYVertices[(i + 1) % IndexCount].Position[1] - ZYVertices[(i - 1 + IndexCount) % IndexCount].Position[1]) / 3, 0.0f, 1.0f };
	}

	if (key3Flag) {
		for (int i = 0; i < IndexCount; i++) {
			Vertex* v1 = new Vertex(), * v2 = new Vertex(), * v3 = new Vertex(), * v4 = new Vertex();
			v1->SetCoords(new float[4] {C[i][0][0], C[i][0][1], 0.0f, 1.0f}), v1->SetColor(new float[4] { 255.0f, 0.0f, 0.0f, 1.0f });
			v2->SetCoords(new float[4] {C[i][1][0], C[i][1][1], 0.0f, 1.0f}), v2->SetColor(new float[4] { 255.0f, 0.0f, 0.0f, 1.0f });
			v3->SetCoords(new float[4] {C[i][2][0], C[i][2][1], 0.0f, 1.0f}), v3->SetColor(new float[4] { 255.0f, 0.0f, 0.0f, 1.0f });
			v4->SetCoords(new float[4] {C[i][3][0], C[i][3][1], 0.0f, 1.0f}), v4->SetColor(new float[4] { 255.0f, 0.0f, 0.0f, 1.0f });
			ZYCRVertices[crInd++] = *v1, ZYCRVertices[crInd++] = *v2, ZYCRVertices[crInd++] = *v3, ZYCRVertices[crInd++] = *v4;
		}

		vector<vector<float>> Points;
		for (auto& a : C) {
			for (auto& b : a) {
				Points.push_back(b);
			}
		}

		int n = Points.size(), cInd = 0;

		for (int pi = 0; pi < IndexCount; pi++) {
			vector<vector<float>> p = { C[pi][0], C[pi][1], C[pi][2], C[pi][3] }, q = p;

			for (float t = T; t <= 1.0f; t += T) {
				for (int k = 1; k < 4; k++) {
					for (int i = 0; i < 4 - k; i++) {
						q[i][0] = (1 - t) * q[i][0] + t * q[i + 1][0];
						q[i][1] = (1 - t) * q[i][1] + t * q[i + 1][1];
					}
				}

				//cout << "Curve point: " << q[0][0] << " " << q[0][1] << endl;
				ZYCurveVertices[cInd].SetCoords(new float[4] { q[0][0], q[0][1], 0.0f, 1.0f });
				ZYCurveVertices[cInd].SetColor(new float[4] { 0.0f, 255.0f, 0.0f, 1.0f });

				float* tangentPoint = bezierTangent(t, p[0], p[1], p[2], p[3]);

				float tx = tangentPoint[0], ty = tangentPoint[1];
				float utx = tx / pow(tx * tx + ty * ty, 0.5), uty = ty / pow(tx * tx + ty * ty, 0.5);
				ZYTangentVertices[cInd].SetCoords(new float[4] {tx + q[0][0], ty + q[0][1], 0.0f, 1.0f});

				float nx = tangentPoint[0] + 1.f * cosf(1.5708), ny = tangentPoint[1] + 1.f * sinf(1.5708);
				float unx = nx / pow(nx * nx + ny * ny, 0.5), uny = ny / pow(nx * nx + ny * ny, 0.5);
				ZYNormalVertices[cInd].SetCoords(new float[4] {nx + q[0][0], ny + q[0][1], 0.0f, 1.0f});

				float bix = tangentPoint[0] + 1.f * cosf(2 * 1.5708), biy = tangentPoint[1] + 1.f * sinf(2 * 1.5708);
				float ubix = bix / pow(bix * bix + biy * biy, 0.5), ubiy = biy / pow(bix * bix + biy * biy, 0.5);
				ZYBiNormalVertices[cInd++].SetCoords(new float[4] {bix + q[0][0], biy + q[0][1], 0.0f, 1.0f});
			}
		}
	}
}

int triangleInd = 0;

void createObjects(void) {
	// ATTN: DERIVE YOUR NEW OBJECTS HERE:  each object has
	// an array of vertices {pos;color} and
	// an array of indices (no picking needed here) (no need for indices)
	// ATTN: Project 1A, Task 1 == Add the points in your scene

	if (initFlag) {

		for (int i = 0; i < IndexCount; i++) {
			float angle = 2 * i * 3.14 / IndexCount;

			Vertices[i].SetCoords(new float[4] {sinf(angle), cosf(angle), 0.0f, 1.0f});
			ZYVertices[i].SetCoords(new float[4] {sinf(angle), cosf(angle), 0.0f, 1.0f});
		}
		/*Vertices[0].SetCoords(new float[4] {0, 0.0f, 0.0f, 1.0f});
		Vertices[1].SetCoords(new float[4] {0.809f, 0.588f, 0.0f, 1.0f});
		Vertices[2].SetCoords(new float[4] {0.501f, 1.541f, 0.0f, 1.0f});
		Vertices[3].SetCoords(new float[4] {-0.501f, 1.541f, 0.0f, 1.0f});
		Vertices[4].SetCoords(new float[4] {-0.809f, 0.588f, 0.0f, 1.0f});
		Vertices[5].SetCoords(new float[4] {0, 0.0f, 0.0f, 1.0f});
		Vertices[6].SetCoords(new float[4] {0.809f, -0.588f, 0.0f, 1.0f});
		Vertices[7].SetCoords(new float[4] {0.501f, -1.541f, 0.0f, 1.0f});
		Vertices[8].SetCoords(new float[4] {-0.501f, -1.541f, 0.0f, 1.0f});
		Vertices[9].SetCoords(new float[4] {-0.809f, -0.588f, 0.0f, 1.0f});

		ZYVertices[0].SetCoords(new float[4] {0, 0.0f, 0.0f, 1.0f});
		ZYVertices[1].SetCoords(new float[4] {0.809f, 0.588f, 0.0f, 1.0f});
		ZYVertices[2].SetCoords(new float[4] {0.501f, 1.541f, 0.0f, 1.0f});
		ZYVertices[3].SetCoords(new float[4] {-0.501f, 1.541f, 0.0f, 1.0f});
		ZYVertices[4].SetCoords(new float[4] {-0.809f, 0.588f, 0.0f, 1.0f});
		ZYVertices[5].SetCoords(new float[4] {0, 0.0f, 0.0f, 1.0f});
		ZYVertices[6].SetCoords(new float[4] {0.809f, -0.588f, 0.0f, 1.0f});
		ZYVertices[7].SetCoords(new float[4] {0.501f, -1.541f, 0.0f, 1.0f});
		ZYVertices[8].SetCoords(new float[4] {-0.501f, -1.541f, 0.0f, 1.0f});
		ZYVertices[9].SetCoords(new float[4] {-0.809f, -0.588f, 0.0f, 1.0f});*/

		for (int i = 0; i < IndexCount; i++) {
			swap(ZYVertices[i].Position[0], ZYVertices[i].Position[1]);
			swap(ZYVertices[i].Position[1], ZYVertices[i].Position[2]);

			Vertices[i].Position[1] += 1;
			ZYVertices[i].Position[1] -= 2.5;


			swap(ZYVertices[i].Position[0], ZYVertices[i].Position[1]);

			cout << "Initial ZY Coordinates: " << ZYVertices[i].Position[0] << ", " << ZYVertices[i].Position[1] << endl;
		}

		P.resize(6);

		for (int i = 0; i < IndexCount; i++) {
			Indices[i] = i;
		}

		for (int i = 0; i < SubIndexCount; i++) {
			SubIndices[i] = i;
		}

		for (int i = 0; i < BBIndexCount; i++) {
			BBIndices[i] = i;
		}

		
		for (int i = 0; i < CRIndexCount; i++) {
			CRIndices[i] = i;
		}

		for (int i = 0; i < CurveIndexCount; i++) {
			CurveIndices[i] = i;
		}

		for (int i = 0; i < 10; i++) {
			P[0].push_back(Vertices[i]);
		}

		for (int i = 0; i < TriangleIndexCount; i++) {
			TriangleIndices[i] = i;
		}

		for (int i = 0; i < TNBIndexCount; i++) {
			TNBIndices[i] = i;
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
	Vertices[5].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[6].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[7].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[8].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	Vertices[9].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });

	ZYVertices[0].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[1].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[2].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[3].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[4].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[5].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[6].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[7].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[8].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });
	ZYVertices[9].SetColor(new float[4] { 1.0f, 1.0f, 1.0f, 1.0f });

	TNBVertices[0].SetColor(new float[4] {255.0f, 0.0f, 0.0f, 1.0f});
	TNBVertices[1].SetColor(new float[4] {255.0f, 0.0f, 0.0f, 1.0f});
	TNBVertices[2].SetColor(new float[4] {0.0f, 255.0f, 0.0f, 1.0f});
	TNBVertices[3].SetColor(new float[4] {0.0f, 255.0f, 0.0f, 1.0f});
	TNBVertices[4].SetColor(new float[4] {0.0f, 0.0f, 255.0f, 1.0f});
	TNBVertices[5].SetColor(new float[4] {0.0f, 0.0f, 255.0f, 1.0f});

	ZYTNBVertices[0].SetColor(new float[4] {255.0f, 0.0f, 0.0f, 1.0f});
	ZYTNBVertices[1].SetColor(new float[4] {255.0f, 0.0f, 0.0f, 1.0f});
	ZYTNBVertices[2].SetColor(new float[4] {0.0f, 255.0f, 0.0f, 1.0f});
	ZYTNBVertices[3].SetColor(new float[4] {0.0f, 255.0f, 0.0f, 1.0f});
	ZYTNBVertices[4].SetColor(new float[4] {0.0f, 0.0f, 255.0f, 1.0f});
	ZYTNBVertices[5].SetColor(new float[4] {0.0f, 0.0f, 255.0f, 1.0f});


	for (int i = 0; i < TriangleIndexCount; i++) {
		TriangleVertices[i].SetColor(new float[4] {255.0f, 255.0f, 0.0f, 1.0f});
		ZYTriangleVertices[i].SetColor(new float[4] {255.0f, 255.0f, 0.0f, 1.0f});
	}

	/*for (int i = 0; i < IndexCount; i++) {
		ZYVertices[i].SetCoords(new float[4] {Vertices[i].Position[1], Vertices[i].Position[2], 0.0f, 1.0f});
	}*/

	/*for (int i = 0; i < IndexCount; i++) {
		for (int j = 0; j < 3; j++) {
			Vertices[i].Position[j] = Vertices[i].Position[j] * 0.5 - 1;
			ZYVertices[i].Position[j] = ZYVertices[i].Position[j] * 0.5 + 1;
		}
	}*/

	for (int i = 0; i < IndexCount; i++) {
		P[0][i] = Vertices[i];
	}

	for (int k = 1; k < 6; k++) {
		int n = P[k - 1].size();
		P[k] = vector<Vertex>();

		for (int i = 0; i < n; i++) {
			std::vector<float> P1Pos = { 0.0f, 0.0f, 0.0f, 1.f };
			std::vector<float> P2Pos = { 0.0f, 0.0f, 0.0f, 1.f };
			std::vector<float> P3Pos = { 0.0f, 0.0f, 0.0f, 1.f };

			//cout << "before index check" << endl;

			P1Pos = { P[k - 1][(i - 1 + n) % n].Position[0], P[k - 1][(i - 1 + n) % n].Position[1], 0.0f, 1.f };
			P2Pos = { P[k - 1][i].Position[0], P[k - 1][i].Position[1], 0.0f, 1.f };
			P3Pos = { P[k - 1][(i + 1) % n].Position[0], P[k - 1][(i + 1) % n].Position[1], 0.0f, 1.f };


			Vertex* v1 = new Vertex(), * v2 = new Vertex();
			v1->SetCoords(new float[4] { 0.5f * (P1Pos[0] + P2Pos[0]), 0.5f * (P1Pos[1] + P2Pos[1]), 0.0f, 1.0f});
			v1->SetColor(new float[4] {0.0f, 100.0f, 100.0f, 1.0f});
			v2->SetCoords(new float[4] { (P1Pos[0] + 6 * P2Pos[0] + P3Pos[0]) / 8, (P1Pos[1] + 6 * P2Pos[1] + P3Pos[1]) / 8, 0.0f, 1.0f});
			v2->SetColor(new float[4] {0.0f, 100.0f, 100.0f, 1.0f});


			P[k].push_back(*v1);
			P[k].push_back(*v2);
		}
	}

	if (key1Flag && k < 6) {
		Vertex* v = new Vertex();
		fill(begin(SubVertices), end(SubVertices), *v);
		int subInd = 0;
		for (int ki = 1; ki <= k && k < 6; ki++) {
			int n = P[ki].size();

			for (int i = 0; i < n; i++) {
				Vertex* v1 = new Vertex();
				v1->SetCoords(new float[4] {P[ki][i].Position[0], P[ki][i].Position[1], 0.0f, 1.0f});
				v1->SetColor(new float[4] {0.0f, 100.0f, 100.0f, 1.0f});

				SubVertices[subInd++] = *v1;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[1], SubVertices, GL_STATIC_DRAW);

	}

	createBB();
	createCR();
	createZYCR();

	if (key5Flag && key3Flag) {
		cout << "orig curve vertices: " << CurveVertices[triangleInd].Position[0] << " " << CurveVertices[triangleInd].Position[1] << " WITH IND " << triangleInd << endl;
		TriangleVertices[0].SetCoords(CurveVertices[triangleInd].Position);
		TriangleVertices[1].SetCoords(new float[4] { TriangleVertices[0].Position[0] + 1.f * cosf(1.0472f), TriangleVertices[0].Position[1] + 1.f * sinf(1.0472f), 0.0f, 1.0f });
		TriangleVertices[2].SetCoords(new float[4] { TriangleVertices[0].Position[0] + 1.f * cosf(2 * 1.0472f), TriangleVertices[0].Position[1] + 1.f * sinf(2 * 1.0472f), 0.0f, 1.0f });

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[8], TriangleVertices, GL_STATIC_DRAW);

		cout << "TAAAAAAAAAAAAAAAAAAAANGENT VERTEX LOOOCATION: " << TangentVertices[triangleInd].Position[0] << " " << TangentVertices[triangleInd].Position[1] << endl;
		TNBVertices[0].SetCoords(new float[4] { TangentVertices[triangleInd].Position[0], TangentVertices[triangleInd].Position[1], 0.0f, 1.0f });

		float x1 = TNBVertices[0].Position[0], y1 = TNBVertices[1].Position[1], x2, y2;
		x2 = CurveVertices[triangleInd].Position[0], y2 = CurveVertices[triangleInd].Position[1];
		float distance = pow(pow(x2 - x1, 2) + pow(y2 - y1, 2), 0.5);

		TNBVertices[1].SetCoords(new float[4] {  x2 , y2 , 0.0f, 1.0f });
		TNBVertices[2].SetCoords(new float[4] { CurveVertices[triangleInd].Position[0], CurveVertices[triangleInd].Position[1], 0.0f, 1.0f });

		x2 = NormalVertices[triangleInd].Position[0], y2 = NormalVertices[triangleInd].Position[1];
		distance = pow(pow(x2 - x1, 2) + pow(y2 - y1, 2), 0.5);

		TNBVertices[3].SetCoords(new float[4] {  x2 , y2 , 0.0f, 1.0f });
		TNBVertices[4].SetCoords(new float[4] { CurveVertices[triangleInd].Position[0], CurveVertices[triangleInd].Position[1], 0.0f, 1.0f });

		x2 = BiNormalVertices[triangleInd].Position[0], y2 = BiNormalVertices[triangleInd].Position[1];
		distance = pow(pow(x2 - x1, 2) + pow(y2 - y1, 2), 0.5);

		TNBVertices[5].SetCoords(new float[4] {  x2 , y2, 0.0f, 1.0f });

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[10]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[10], TNBVertices, GL_STATIC_DRAW);
		/*TNBVertices[2].SetCoords(new float[4] { CurveVertices[(triangleInd - 1) % CurveIndexCount].Position[0], CurveVertices[(triangleInd - 1) % CurveIndexCount].Position[1], 0.0f, 1.0f });
		TNBVertices[3].SetCoords(new float[4] { CurveVertices[(triangleInd - 1) % CurveIndexCount].Position[0], CurveVertices[(triangleInd - 1) % CurveIndexCount].Position[1], 0.0f, 1.0f });
		TNBVertices[4].SetCoords(new float[4] { CurveVertices[(triangleInd - 1) % CurveIndexCount].Position[0], CurveVertices[(triangleInd - 1) % CurveIndexCount].Position[1], 0.0f, 1.0f });
		TNBVertices[5].SetCoords(new float[4] { CurveVertices[(triangleInd - 1) % CurveIndexCount].Position[0], CurveVertices[(triangleInd - 1) % CurveIndexCount].Position[1], 0.0f, 1.0f });*/


		if (key4Flag) {
			ZYTriangleVertices[0].SetCoords(ZYCurveVertices[triangleInd].Position);
			ZYTriangleVertices[1].SetCoords(new float[4] { ZYTriangleVertices[0].Position[0] + 1.f * cosf(1.0472f), ZYTriangleVertices[0].Position[1] + 1.f * sinf(1.0472f), 0.0f, 1.0f });
			ZYTriangleVertices[2].SetCoords(new float[4] { ZYTriangleVertices[0].Position[0] + 1.f * cosf(2 * 1.0472f), ZYTriangleVertices[0].Position[1] + 1.f * sinf(2 * 1.0472f), 0.0f, 1.0f });
			
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[9]);
			glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[9], ZYTriangleVertices, GL_STATIC_DRAW);

			ZYTNBVertices[0].SetCoords(new float[4] { ZYCurveVertices[triangleInd].Position[0], ZYCurveVertices[triangleInd].Position[1], 0.0f, 1.0f });
			ZYTNBVertices[1].SetCoords(new float[4] {  ZYTangentVertices[triangleInd].Position[0], ZYTangentVertices[triangleInd].Position[1], 0.0f, 1.0f });
			ZYTNBVertices[2].SetCoords(new float[4] { ZYCurveVertices[triangleInd].Position[0], ZYCurveVertices[triangleInd].Position[1], 0.0f, 1.0f });
			ZYTNBVertices[3].SetCoords(new float[4] {  ZYNormalVertices[triangleInd].Position[0], ZYNormalVertices[triangleInd].Position[1], 0.0f, 1.0f });
			ZYTNBVertices[4].SetCoords(new float[4] { ZYCurveVertices[triangleInd].Position[0], ZYCurveVertices[triangleInd].Position[1], 0.0f, 1.0f });
			ZYTNBVertices[5].SetCoords(new float[4] { ZYBiNormalVertices[triangleInd].Position[0], ZYBiNormalVertices[triangleInd].Position[1], 0.0f, 1.0f });

			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[11]);
			glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[11], ZYTNBVertices, GL_STATIC_DRAW);
		}

		triangleInd = (triangleInd + 1) % CurveIndexCount;

		if (CurveVertices[triangleInd].Position[0] == 0.0f && CurveVertices[triangleInd].Position[1] == 0.0f)
			triangleInd = 1;

		cout << "setting triangle vertices: " << TriangleVertices[0].Position[0] << " " << TriangleVertices[0].Position[1] << ", " <<
			TriangleVertices[1].Position[0] << " " << TriangleVertices[1].Position[1] << ", " <<
			TriangleVertices[1].Position[0] << " " << TriangleVertices[1].Position[1] << endl;
	}

	//frameCount = (frameCount + 1) % 2;

	/*else {
		TriangleVertices[0].SetCoords(CurveVertices[0].Position);
		TriangleVertices[1].SetCoords(ZYCurveVertices[0].Position);
		triangleInd = 0;
	}*/
	/*if(key3Flag) 
		bindBuffers();*/

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

	if (gPickedIndex < IndexCount) {
		cout << "click: " << Vertices[gPickedIndex].Position[2] << endl;
	}
	
	// Uncomment these lines if you wan to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the visible rendering
}

// ATTN: Project 1A, Task 3 == Retrieve your cursor position, get corresponding world coordinate, and move the point accordingly

// ATTN: Project 1C, Task 1 == Keep track of z coordinate for selected point and adjust its value accordingly based on if certain
// buttons are being pressed

float* worldCoords = new float[4] {0.0f, 0.0f, 0.0f, 1.0f}; //to be used in moveVertex and mouseCallback

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
	if (gPickedIndex < IndexCount) {
		if (shiftFlag) {
			worldCoords[2] += -(Vertices[gPickedIndex].Position[1] - worldCoords[1]);
			worldCoords[1] = Vertices[gPickedIndex].Position[1];
		}

		else {
			worldCoords[2] = Vertices[gPickedIndex].Position[2];
		}

		Vertices[gPickedIndex].SetCoords(worldCoords);
		Vertices[gPickedIndex].SetColor(newColor);

		ZYVertices[gPickedIndex].SetCoords(new float[4] {-worldCoords[1], worldCoords[2] - 2.5f, 0.0f, 1.0f});
		ZYVertices[gPickedIndex].SetColor(newColor);

		swap(ZYVertices[gPickedIndex].Position[0], ZYVertices[gPickedIndex].Position[1]);
		cout << "ZY coordinates of "<<gPickedIndex<<" : " << ZYVertices[gPickedIndex].Position[0] << ", " << ZYVertices[gPickedIndex].Position[1] << endl;
	}

	bindBuffers();

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

	/*glBegin(GL_TRIANGLES);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glVertex4f(0.0f, 0.0f, 0.0f, 1.0f);
	glVertex4f(1.0f, 1.0f, 0.0f, 1.0f);
	glVertex4f(1.0f, -1.0f, 0.0f, 1.0f);
	glEnd();
	glFlush();*/

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
		glDrawElements(GL_LINE_LOOP, NumIdcs[0], GL_UNSIGNED_SHORT, (void*)0);
		// // If don't use indices
		// glDrawArrays(GL_POINTS, 0, NumVerts[0]);	

		// ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE
		// one set per object:
		// glBindVertexArray(VertexArrayId[<x>]); etc etc
		
		if (key1Flag) {
			glBindVertexArray(VertexArrayId[1]);	// Draw Vertices
			glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[1], SubVertices);		// Update buffer data
			glDrawElements(GL_POINTS, NumIdcs[1], GL_UNSIGNED_SHORT, (void*)0);
		}

		if (key2Flag) {
			glBindVertexArray(VertexArrayId[2]);	// Draw Vertices
			glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[2], BBVertices);		// Update buffer data
			glDrawElements(GL_POINTS, NumIdcs[2], GL_UNSIGNED_SHORT, (void*)0);
		}

		if (key3Flag) {
			glBindVertexArray(VertexArrayId[3]);	// Draw Vertices
			glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[3], CRVertices);
			glDrawElements(GL_POINTS, NumIdcs[3], GL_UNSIGNED_SHORT, (void*)0);
			glBindVertexArray(VertexArrayId[4]);	// Draw Vertices
			glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[4], CurveVertices);		// Update buffer data
			glDrawElements(GL_LINE_LOOP, NumIdcs[4], GL_UNSIGNED_SHORT, (void*)0);
		}

		if (key4Flag) {
			glBindVertexArray(VertexArrayId[5]);	// Draw Vertices
			glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[5], ZYVertices);
			glDrawElements(GL_POINTS, NumIdcs[5], GL_UNSIGNED_SHORT, (void*)0);
			glDrawElements(GL_LINE_LOOP, NumIdcs[5], GL_UNSIGNED_SHORT, (void*)0);

			if (key3Flag) {
				glBindVertexArray(VertexArrayId[6]);	// Draw Vertices
				glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[6], ZYCRVertices);
				glDrawElements(GL_POINTS, NumIdcs[6], GL_UNSIGNED_SHORT, (void*)0);
				glBindVertexArray(VertexArrayId[7]);	// Draw Vertices
				glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[7], ZYCurveVertices);		// Update buffer data
				glDrawElements(GL_LINE_LOOP, NumIdcs[7], GL_UNSIGNED_SHORT, (void*)0);
			}

			if (key5Flag) {
				cout << "RENDERING TRAINGLE KEY5: " << key5Flag << " AND KEY3FLAG: " << key3Flag << endl;
				cout << "TRIANGLE VERTEX: " << TriangleVertices[0].Position[1] << endl;
				cout << "RENDERING XY TRIANGLES" << endl;
				//glBindVertexArray(VertexArrayId[8]);	// Draw Vertices
				//glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[8], TriangleVertices);
				//glDrawElements(GL_LINE_LOOP, NumIdcs[8], GL_UNSIGNED_SHORT, (void*)0);

				glBindVertexArray(VertexArrayId[10]);	// Draw Vertices
				glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[10], TNBVertices);
				glDrawElements(GL_LINES, NumIdcs[10], GL_UNSIGNED_SHORT, (void*)0);

				if (key4Flag) {
					cout << "RENDERING ZY TRIANGLES" << endl;
					//glBegin(GL_TRIANGLES);
					//glColor3f(1.0f, 1.0f, 1.0f);
					///*glVertex2i(ZYTriangleVertices[0].Position[0], ZYTriangleVertices[0].Position[1]);
					//glVertex2i(ZYTriangleVertices[1].Position[0], ZYTriangleVertices[1].Position[1]);
					//glVertex2i(ZYTriangleVertices[2].Position[0], ZYTriangleVertices[2].Position[1]);*/
					//glVertex2i(0.0f, 0.0f);
					//glVertex2i(1.0f, 1.0f);
					//glVertex2i(1.0f, -1.0f);
					//glEnd();
					//glFlush();
					//glBindVertexArray(VertexArrayId[9]);	// Draw Vertices
					//glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[9], ZYTriangleVertices);
					////glDrawArrays(GL_, 0, 3);
					//glDrawElements(GL_LINE_LOOP, NumIdcs[9], GL_UNSIGNED_SHORT, (void*)0);

					glBindVertexArray(VertexArrayId[11]);	// Draw Vertices
					glBufferSubData(GL_ARRAY_BUFFER, 0, VertexBufferSize[11], ZYTNBVertices);
					glDrawElements(GL_LINES, NumIdcs[11], GL_UNSIGNED_SHORT, (void*)0);
				}
			}
		}

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
		if (gPickedIndex < IndexCount) {
			if (shiftFlag) {
				worldCoords[2] += -(Vertices[gPickedIndex].Position[1] - worldCoords[1]);
				worldCoords[1] = Vertices[gPickedIndex].Position[1];
			}
			Vertices[gPickedIndex].SetColor(prevColor);
			Vertices[gPickedIndex].SetCoords(worldCoords);

			ZYVertices[gPickedIndex].SetCoords(new float[4] {-worldCoords[1], worldCoords[2] - 2.5f, 0.0f, 1.0f});
			ZYVertices[gPickedIndex].SetColor(prevColor);

			swap(ZYVertices[gPickedIndex].Position[0], ZYVertices[gPickedIndex].Position[1]);

			cout << "release: " << Vertices[gPickedIndex].Position[2] << endl;

			cout << "ZY coordinates of " << gPickedIndex << " : " << ZYVertices[gPickedIndex].Position[0] << ", " << ZYVertices[gPickedIndex].Position[1] << endl;

		}

		bindBuffers();

		/*glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[0], Vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[1], SubVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[2], BBVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[3], CRVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[4], CurveVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[5], ZYVertices, GL_STATIC_DRAW);*/
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		key1Flag = true;
		subDivide();
		key1Flag = true;
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
		glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[1], SubVertices, GL_STATIC_DRAW);
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		key2Flag = !key2Flag;
		createBB();
		if (key2Flag) {
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);  
			glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[2], BBVertices, GL_STATIC_DRAW);
		}
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		key3Flag = !key3Flag;
		createCR(); 
		if (key3Flag) {
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
			glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[3], CRVertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
			glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[4], CurveVertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
			glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[6], ZYCRVertices, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
			glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[7], ZYCurveVertices, GL_STATIC_DRAW);
		}
	}

	if ((key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) && action == GLFW_PRESS) {
		shiftFlag = !shiftFlag;
	}

	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		key4Flag = !key4Flag;
	}

	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
		key5Flag = true;
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
