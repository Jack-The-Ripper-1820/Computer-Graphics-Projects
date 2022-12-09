// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
#include <iostream>
#include <unordered_map>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
using namespace glm;
using namespace std;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/quaternion_utils.hpp>
#include <common/texture.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <common/stb_image.h>

const int window_width = 600, window_height = 600;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	float UV[2];

	void SetPosition(float* coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float* color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetUV(float* coords) {
		UV[0] = coords[0];
		UV[1] = coords[1];
	}
	void SetNormal(float* coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], GLushort[], int);
void loadObject(char*, glm::vec4, Vertex*&, GLushort*&, size_t&, int);
void createObjects(void);
void pickObject(void);
void renderScene(float);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
void updateLight();
void genPNTrianglesAndQuads(Vertex*, size_t&);
void addTexture(int);


// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;
GLuint tessProgramID;
GLuint tessQuadProgramID;

const GLuint NumObjects = 4;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects];
GLuint VertexBufferId[NumObjects];
GLuint IndexBufferId[NumObjects];
GLuint TextureBufferId[NumObjects];

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

GLuint TessModelMatrixID;
GLuint TessViewMatrixID;
GLuint TessProjectionMatrixID;
GLuint TessLightID;
GLfloat TessLevelID;
GLfloat TessLevelQuadID;
GLuint TessModelMatrixQuadID;
GLuint TessViewMatrixQuadID;
GLuint TessProjectionMatrixQuadID;
GLuint TessLightQuadID;

float TessLvl = 3.f;

int width, height, nrChannels;
unsigned char* Data;

glm::vec3 cameraPos, worldUp;

// Declare global objects
// TL
const size_t CoordVertsCount = 6;
Vertex CoordVerts[CoordVertsCount];

const size_t GridVertsCount = 12 * 12;
Vertex GridVerts[GridVertsCount];
GLushort GridIndices[GridVertsCount];

size_t FaceVertCount, FaceIndexCount;
Vertex* FaceVerts;
GLushort* FaceIndices;

bool cPress = false, rPress = false, genTriangles = false, fPress = false, tessFlag = false, uPress = false;

const size_t NewFaceVertCount = 70000;
Vertex NewFaceVerts[NewFaceVertCount];
GLushort NewFaceIndices[NewFaceVertCount];

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
	TwBar* GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
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

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix

	gViewMatrix = glm::lookAt(cameraPos,	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		worldUp);	// up

	updateLight();

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("P3_StandardShading.vertexshader", "P3_StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("P3_Picking.vertexshader", "P3_Picking.fragmentshader");
	tessProgramID = LoadTessShaders("tess.vs.glsl", "tess.tc.glsl", "tess.te.glsl", "tess.fs.glsl");
	tessQuadProgramID = LoadTessShaders("tessquad.vs.glsl", "tessquad.tc.glsl", "tessquad.te.glsl", "tessquad.fs.glsl");
	
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

	TessModelMatrixID = glGetUniformLocation(tessProgramID, "M");
	TessViewMatrixID = glGetUniformLocation(tessProgramID, "V");
	TessProjectionMatrixID = glGetUniformLocation(tessProgramID, "P");
	TessLightID = glGetUniformLocation(tessProgramID, "lightPosition_worldspace");
	TessLevelID = glGetUniformLocation(tessProgramID, "TessellationLevel");
	TessModelMatrixQuadID = glGetUniformLocation(tessProgramID, "M");
	TessViewMatrixQuadID = glGetUniformLocation(tessProgramID, "V");
	TessProjectionMatrixQuadID = glGetUniformLocation(tessProgramID, "P");
	TessLightQuadID = glGetUniformLocation(tessProgramID, "lightPosition_worldspace");
	TessLevelQuadID = glGetUniformLocation(tessProgramID, "TessellationLevel");
	//TessLevelQuadID = glGetUniformLocation(tessQuadProgramID, "TessellationLevel");
	
	//Texture = loadBMP_custom("raiden-face.BMP");
	Data = stbi_load("Face-Color.jpg", &width, &height, &nrChannels, 0);
	//Data = stbi_load("trialmap.jpg", &width, &height, &nrChannels, 0);
	//GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// TL
	// Define objects
	createObjects();

	// ATTN: create VAOs for each of the newly created objects here:
	VertexBufferSize[0] = sizeof(CoordVerts);
	NumVerts[0] = CoordVertsCount;

	VertexBufferSize[1] = sizeof(GridVerts);
	NumVerts[1] = GridVertsCount;

	VertexBufferSize[3] = sizeof(NewFaceVerts);
	NumVerts[3] = NewFaceVertCount;

	createVAOs(CoordVerts, NULL, 0);
	createVAOs(GridVerts, NULL, 1);
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {
	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;
	const size_t UvOffset = sizeof(Vertices[0].Normal) + Normaloffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);
	glBindVertexArray(VertexArrayId[ObjectId]);
	/*glGenTextures(1, &RenderedTexture[ObjectId]);
	glBindTexture(GL_TEXTURE_2D, RenderedTexture[ObjectId]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/

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

	glGenTextures(1, &TextureBufferId[ObjectId]);
	//glActiveTexture(GL_TEXTURE0);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, TextureBufferId[ObjectId]);

	// Give the image to OpenGL
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_BGR, GL_UNSIGNED_BYTE, 0);

	//glUniform1i(TextureID, 0);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (uPress) {
		if (Data)
		{
			cout << "for objectId: " << ObjectId << endl;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
	}

	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);	// TL
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)UvOffset);


	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal
	glEnableVertexAttribArray(3); // uv

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

void addTexture(int ObjectId) {

	glGenTextures(1, &TextureBufferId[ObjectId]);
	//glActiveTexture(GL_TEXTURE0);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, TextureBufferId[ObjectId]);

	// Give the image to OpenGL
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_BGR, GL_UNSIGNED_BYTE, 0);

	//glUniform1i(TextureID, 0);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (uPress) {
		if (Data)
		{
			cout << "for objectId: " << ObjectId << endl;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
	}

	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

}

// Ensure your .obj files are in the correct format and properly loaded by looking at the following function
void loadObject(char* file, glm::vec4 color, Vertex*& out_Vertices, GLushort*& out_Indices, size_t& VertCount, size_t &IndexCount, int ObjectId) {
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, uvs, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();
	VertCount = vertCount;
	IndexCount = idxCount;

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
		out_Vertices[i].SetUV(&indexed_uvs[i].x);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
		if(i + 1 < idxCount && indices[i + 1] - indices[i] != 1) {
			//cout << indices[i] << " " << indices[i + 1] << endl;
		}
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

		f = !f;
		ind++;
	}

	f = true;
	while (x <= 5.0) {
		if (f)
			GridVerts[ind] = { { x, 0.0, -5.0, 1.0 }, { 255.0, 255.0, 255.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		else
			GridVerts[ind] = { { x, 0.0, 5.0, 1.0}, {255.0, 255.0, 255.0, 1.0}, {0.0, 0.0, 1.0} }, x++;

		f = !f;
		ind++;
	}

	for (int i = 0; i < GridVertsCount; i++) {
		GridIndices[i] = i;
	}

	/*for (int i = 0; i < NewFaceVertCount; i++) {
		NewFaceIndices[i] = i;
	}*/

	//-- .OBJs --//
	// ATTN: Load your models here through .obj files -- example of how to do so is as shown
	// Vertex* Verts;
	// GLushort* Idcs;
	// loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, ObjectID);
	// createVAOs(Verts, Idcs, ObjectID);

	Vertex* Verts;
	GLushort* Idcs;
	size_t VertCount, IdxCount;
	loadObject("models/face-stencil.obj", glm::vec4(1), Verts, Idcs, VertCount, IdxCount, 2);
	//loadObject("models/HeadWithTexture.obj", glm::vec4(1), Verts, Idcs, VertCount, IdxCount, 2);
	createVAOs(Verts, Idcs, 2);
	FaceVerts = Verts;
	FaceVertCount = VertCount;
	FaceIndices = Idcs;
	FaceIndexCount = IdxCount;
	cout << "object successfully loaded with VertCount : " << VertCount << endl;

	//genPNTrianglesAndQuads(Verts, VertCount);
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
		oss << "mesh " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void genPNTrianglesAndQuads(Vertex* Verts, size_t &IndexCount) {
	//if (!genTriangles) return;

	genTriangles = false;
	int ind = 0;

	/*vector<vec3> prevPos, prevNorms;
	for (int i = 0; i < IndexCount; i++) {
		vec3 P1 = vec3(Verts[i].Position[0], Verts[i].Position[1], Verts[i].Position[2]);
		vec3 N1 = vec3(Verts[i].Normal[0], Verts[i].Normal[1], Verts[i].Normal[2]);
		prevPos.push_back(P1);
		prevNorms.push_back(N1);
	}*/

	for (int i = 0; i < IndexCount; i++) {
		int first = i, second = (i + 1) % IndexCount, third = (i + 2) % IndexCount, fourth = (i + 3) % IndexCount;
		
		vec3 P1 = vec3(Verts[first].Position[0], Verts[first].Position[1], Verts[first].Position[2]);
		vec3 P2 = vec3(Verts[second].Position[0], Verts[second].Position[1], Verts[second].Position[2]);
		vec3 P3 = vec3(Verts[third].Position[0], Verts[third].Position[1], Verts[third].Position[2]);
		vec3 P4 = vec3(Verts[fourth].Position[0], Verts[fourth].Position[1], Verts[fourth].Position[2]);

		vec3 N1 = vec3(Verts[first].Normal[0], Verts[first].Normal[1], Verts[first].Normal[2]);
		vec3 N2 = vec3(Verts[second].Normal[0], Verts[second].Normal[1], Verts[second].Normal[2]);
		vec3 N3 = vec3(Verts[third].Normal[0], Verts[third].Normal[1], Verts[third].Normal[2]);
		vec3 N4 = vec3(Verts[fourth].Normal[0], Verts[fourth].Normal[1], Verts[fourth].Normal[2]);

		vec2 UV1 = vec2(Verts[first].UV[0], Verts[first].UV[1]);
		vec2 UV2 = vec2(Verts[second].UV[0], Verts[second].UV[1]);
		vec2 UV3 = vec2(Verts[third].UV[0], Verts[third].UV[1]);
		vec2 UV4 = vec2(Verts[fourth].UV[0], Verts[fourth].UV[1]);

		vec3 C1 = vec3(Verts[first].Color[0], Verts[first].Color[1], Verts[first].Color[2]);
		vec3 C2 = vec3(Verts[second].Color[0], Verts[second].Color[1], Verts[second].Color[2]);
		vec3 C3 = vec3(Verts[third].Color[0], Verts[third].Color[1], Verts[third].Color[2]);
		vec3 C4 = vec3(Verts[fourth].Color[0], Verts[fourth].Color[1], Verts[fourth].Color[2]);

		vec3 b300 = P1, b030 = P2, b003 = P3;
		float w12 = dot((P2 - P1), N1), w21 = dot((P1 - P2), N2), w23 = dot((P3 - P2), N2), w32 = dot((P2 - P3), N3), w13 = dot((P3 - P1), N1), w31 = dot((P1 - P3), N3);

		vec3 b210 = (2.f * P1 + P2 - w12 * N1) / 3.f;
		vec3 b120 = (2.f * P2 + P1 - w21 * N2) / 3.f;
		vec3 b021 = (2.f * P2 + P3 - w23 * N2) / 3.f;
		vec3 b012 = (2.f * P3 + P2 - w32 * N3) / 3.f;
		vec3 b102 = (2.f * P3 + P1 - w31 * N3) / 3.f;
		vec3 b201 = (2.f * P1 + P3 - w13 * N1) / 3.f;

		vec3 E = (b210 + b120 + b021 + b012 + b102 + b201) / 6.f;
		vec3 V = (P1 + P2 + P3) / 3.f;

		vec3 b111 = E + (E - V) / 2.f;

		vec3 n200 = N1, n020 = N2, n002 = N3;
		float v12 = 2.f * dot((P2 - P1), (N1 + N2)) / dot((P2 - P1), (P2 - P1));
		float v23 = 2.f * dot((P3 - P2), (N2 + N3)) / dot((P3 - P2), (P3 - P2));
		float v31 = 2.f * dot((P3 - P1), (N3 + N1)) / dot((P3 - P1), (P3 - P1));
		
		vec3 h110 = N1 + N2 - v12 * (P2 - P1), h011 = N2 + N3 - v23 * (P3 - P2), h101 = N3 + N1 - v31 * (P1 - P3);
		vec3 n110 = normalize(h110), n011 = normalize(h011), n101 = normalize(h101);

		//float u = Verts[i].UV[0], v = Verts[i].UV[1], w = 1 - u - v;
		
		vec2 avgUV = (UV1 + UV2 + UV3) / 3.f;
		vec3 avgColor = (C1 + C2 + C3) / 3.f;

		//cout << "before loop: " << ind<< endl;
		for (float u = 0; u <= 1; u += 0.4) {
			for (float v = 0; u + v <= 1; v += 0.4) {
				float w = 1 - u - v;
				vec3 buv = b300 * w * w * w + b030 * u * u * u + b003 * v * v * v + b210 * 3.f * w * w * u
					+ b120 * 3.f * w * u * u + b201 * 3.f * w * w * v + b021 * 3.f * u * u * v + b102 * 3.f * w * v * v
					+ b012 * 3.f * u * v * v + b111 * 6.f * w * v * u;

				vec3 nuv = n200 * w * w + n020 * u * u + n002 * v * v + n110 * w * u + n011 * u * v + n101 * w * v;

				NewFaceVerts[ind].SetPosition(new float[4] {buv[0], buv[1], buv[2], 1});
				//cout << buv[0] << " " << buv[1] << " " << buv[2] << endl;
				NewFaceVerts[ind].SetNormal(new float[3] {nuv[0], nuv[1], nuv[2]});
				//NewFaceVerts[ind].SetColor(new float[4] {avgColor[0], avgColor[1], avgColor[2], 1});
				NewFaceVerts[ind].SetColor(new float[4] {avgColor[0], avgColor[1], avgColor[2], 1});
				NewFaceVerts[ind].SetUV(new float[2] {avgUV[0], avgUV[1]});

				NewFaceIndices[ind] = FaceIndices[first] + ind;
				ind++;
			}
		}

		cout << "PRE QUAD IND: " << ind << endl;

		//quad calculation

		vec3 b0 = P1, b1 = P2, b3 = P3, b4 = P4, n0 = N1, n1 = N2, n2 = N3, n3 = N4;
		vector<vector<vec3>> b(4, vector<vec3>(4));

		vec3 b01 = (2.f * P1 + P2 - dot(P2 - P1, N1) * N1) / 3.f;
		vec3 b32 = (2.f * P4 + P3 - dot(P3 - P4, N4) * N4) / 3.f;
		vec3 b03 = (2.f * P1 + P4 - dot(P4 - P1, N1) * N1) / 3.f;
		vec3 b30 = (2.f * P1 + P4 - dot(P1 - P4, N4) * N4) / 3.f;
		vec3 b10 = (2.f * P2 + P1 - dot(P1 - P2, N2) * N2) / 3.f;
		vec3 b23 = (2.f * P3 + P4 - dot(P4 - P3, N3) * N3) / 3.f;
		vec3 b12 = (2.f * P2 + P3 - dot(P3 - P2, N2) * N2) / 3.f;
		vec3 b21 = (2.f * P3 + P2 - dot(P2 - P3, N3) * N3) / 3.f;

		float v01 = 2.f * dot((P2 - P1), (N1 + N2)) / dot((P2 - P1), (P2 - P1));
		float v12q = 2.f * dot((P3 - P2), (N2 + N3)) / dot((P3 - P2), (P3 - P2));
		float v23q = 2.f * dot((P4 - P3), (P3 + P4)) / dot((P4 - P3), (P4 - P3));
		float v30 = 2.f * dot((P1 - P4), (P4 + P1)) / dot((P1 - P4), (P1 - P4));

		vec3 h01 = N1 + N2 - v01 * (P2 - P1);
		vec3 h12 = N2 + N3 - v12q * (P3 - P2);
		vec3 h23 = N3 + N4 - v23q * (P4 - P3);
		vec3 h30 = N4 + N1 - v30 * (N1 - N4);

		vec3 n01 = normalize(h01), n12 = normalize(h12), n23 = normalize(h23), n30 = normalize(h30);
		vec3 q = b03 + b01 + b10 + b12 + b21 + b23 + b32 + b30;
		
		vec3 E0 = (2.f * (b01 + b03 + q) - (b21 + b23)) / 18.f;
		vec3 V0 = ((4.f * P1) + 2.f * (P4 + P2) + P3) / 9.f;

		vec3 E1 = (2.f * (b12 + b10 + q) - (b32 + b30)) / 18.f;
		vec3 V1 = ((4.f * P2) + 2.f * (P1 + P3) + P4) / 9.f;

		vec3 E2 = (2.f * (b23 + b21 + q) - (b03 + b01)) / 18.f;
		vec3 V2 = ((4.f * P3) + 2.f * (P2 + P4) + P1) / 9.f;

		vec3 E3 = (2.f * (b30 + b32 + q) - (b10 + b12)) / 18.f;
		vec3 V3 = ((4.f * P4) + 2.f * (P3 + P1) + P2) / 9.f;
		
		float sigma = 0.5f;

		vec3 b02 = (1 + sigma) * E0 - sigma * V0;
		vec3 b31 = (1 + sigma) * E3 - sigma * E3;
		vec3 b13 = (1 + sigma) * E1 - sigma * E1;
		vec3 b20 = (1 + sigma) * E2 - sigma * E2;

		vec3 n0123 = (2.f * (n01 + n12 + n23 + n30) + (n0 + n1 + n2 + n3)) / 12.f;

		vec3 avgColorQuad = (C1 + C2 + C3 + C4) / 4.f;
		vec2 avgUVQuad = (UV1 + UV2 + UV3 + UV4) / 4.f;

		NewFaceVerts[ind].SetPosition(new float[4] {b02[0], b02[1], b02[2], 1});
		NewFaceVerts[ind].SetNormal(new float[3] {n0123[0], n0123[1], n0123[2]});
		NewFaceVerts[ind].SetColor(new float[4] {avgColorQuad[0], avgColorQuad[1], avgColorQuad[2], 1});
		NewFaceVerts[ind].SetUV(new float[2] {avgUVQuad[0], avgUVQuad[1]});
		
		NewFaceIndices[ind] = FaceIndices[first] + ind;
		ind++;

		NewFaceVerts[ind].SetPosition(new float[4] {b31[0], b31[1], b31[2], 1});
		NewFaceVerts[ind].SetNormal(new float[3] {n0123[0], n0123[1], n0123[2]});
		NewFaceVerts[ind].SetColor(new float[4] {avgColorQuad[0], avgColorQuad[1], avgColorQuad[2], 1});
		NewFaceVerts[ind].SetUV(new float[2] {avgUVQuad[0], avgUVQuad[1]});

		NewFaceIndices[ind] = FaceIndices[first] + ind;
		ind++;

		NewFaceVerts[ind].SetPosition(new float[4] {b13[0], b13[1], b13[2], 1});
		NewFaceVerts[ind].SetNormal(new float[3] {n0123[0], n0123[1], n0123[2]});
		NewFaceVerts[ind].SetColor(new float[4] {avgColorQuad[0], avgColorQuad[1], avgColorQuad[2], 1});
		NewFaceVerts[ind].SetUV(new float[2] {avgUVQuad[0], avgUVQuad[1]});

		NewFaceIndices[ind] = FaceIndices[first] + ind;
		ind++;

		NewFaceVerts[ind].SetPosition(new float[4] {b20[0], b20[1], b20[2], 1});
		NewFaceVerts[ind].SetNormal(new float[3] {n0123[0], n0123[1], n0123[2]});
		NewFaceVerts[ind].SetColor(new float[4] {avgColorQuad[0], avgColorQuad[1], avgColorQuad[2], 1});
		NewFaceVerts[ind].SetUV(new float[2] {avgUVQuad[0], avgUVQuad[1]});

		NewFaceIndices[ind] = FaceIndices[first] + ind;
		ind++;

		cout << "POST QUAD IND: " << ind << endl;
	}

	cout << "ind: " << ind << endl;
	NumIdcs[3] = NewFaceVertCount;
	VertexBufferSize[3] = sizeof(NewFaceVerts[0]) * NewFaceVertCount;
	IndexBufferSize[3] = sizeof(GLushort) * NewFaceVertCount;
	createVAOs(NewFaceVerts, NewFaceIndices, 3);

	cout << NewFaceVerts[0].Position[0] << " " << NewFaceVerts[0].Position[1] << " " << NewFaceVerts[0].Position[2] << endl;
	/*IndexCount = prevPos.size();
	const int NewIndexCount = IndexCount;
	Vertex NewVerts[5000];

	for (int i = 0; i < IndexCount; i++) {
		NewVerts[i].SetPosition(new float[4] {prevPos[i][0], prevPos[i][1], prevPos[i][2], 1});
		NewVerts[i].SetNormal(new float[4] {prevNorms[i][0], prevNorms[i][1], prevNorms[i][2], 1});
		NewVerts[i].SetColor(new float[4] {Verts[i][0], prevPos[i][1], prevPos[i][2], 1});
		NewVerts[i].SetPosition(new float[4] {prevPos[i][0], prevPos[i][1], prevPos[i][2], 1});
	}

	Verts = NewVerts;*/
}

void renderScene(float deltaTime) {
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4x4 ModelMatrix = glm::mat4(1.0);
	glm::vec3 lightPos = vec3(cameraPos.x - 2, cameraPos.y, cameraPos.z - 2);
	vec3 lightPos2 = vec3(cameraPos.x + 2, cameraPos.y, cameraPos.z - 2);
	vec3 lightPosArray[2] = { lightPos, lightPos2 };

	if (fPress) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glUseProgram(programID);
	{	
		glUniform3fv(LightID, 2, (GLfloat*)lightPosArray);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[0]);	// Draw CoordAxes
		glDrawArrays(GL_LINES, 0, NumVerts[0]);

		glBindVertexArray(VertexArrayId[1]);	// Draw Grid
		glDrawArrays(GL_LINES, 0, NumVerts[1]);
		
		//glBindTexture(GL_TEXTURE_2D, TextureBufferId[2]);
		glBindVertexArray(VertexArrayId[2]);	// Draw Vertices

		glDrawElements(GL_TRIANGLES, NumIdcs[2], GL_UNSIGNED_SHORT, (void*)0);		

		glBindVertexArray(0);
	}

	if (genTriangles) {
		glUseProgram(tessProgramID);
		{
			glUniform3fv(TessLightID, 2, (GLfloat*)lightPosArray);
			glUniformMatrix4fv(TessViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
			glUniformMatrix4fv(TessProjectionMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
			glUniformMatrix4fv(TessModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			glUniform1f(TessLevelID, TessLvl);

			glPatchParameteri(GL_PATCH_VERTICES, 3);
			glBindVertexArray(VertexArrayId[2]);
			glDrawElements(GL_PATCHES, NumIdcs[2], GL_UNSIGNED_SHORT, (void*)0);

			glBindVertexArray(0);
		}

		
		/*glUseProgram(tessQuadProgramID);
		{
			glUniform3fv(TessLightQuadID, 2, (GLfloat*)lightPosArray);
			glUniformMatrix4fv(TessViewMatrixQuadID, 1, GL_FALSE, &gViewMatrix[0][0]);
			glUniformMatrix4fv(TessProjectionMatrixQuadID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
			glUniformMatrix4fv(TessModelMatrixQuadID, 1, GL_FALSE, &ModelMatrix[0][0]);

			glUniform1f(TessLevelQuadID, TessLvl);

			glPatchParameteri(GL_PATCH_VERTICES, 4);
			glBindVertexArray(VertexArrayId[2]);
			glDrawElements(GL_PATCHES, NumIdcs[2], GL_UNSIGNED_SHORT, (void*)0);

			glBindVertexArray(0);
		}*/
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void cleanup(void) {
	//glDeleteTextures(1, &texture);
	// Cleanup VBO and shader
	stbi_image_free(Data);
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

void resetCamera() {
	gViewMatrix = glm::lookAt(vec3(10.0f, 10.0f, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		worldUp);
}

float prevX = 10.f, prevZ = 10.f, prevY = 10.f;

// Alternative way of triggering functions on keyboard events
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_C:
			cPress = !cPress;
			break;
		case GLFW_KEY_R:
			rPress = !rPress;
			resetCamera();
			break;
		case GLFW_KEY_F:
			fPress = !fPress;
			break;
		case GLFW_KEY_P:
			genTriangles = !genTriangles;
			break;
		case GLFW_KEY_U:
			uPress = !uPress;
			addTexture(2);
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

float theta = atan(1), phi = atan(1);
const float radius = 10.0f;

void updateLight() {
	glUseProgram(programID);
	{
		glm::vec3 lightPos = vec3(cameraPos.x - 2, cameraPos.y, cameraPos.z - 2);
		vec3 lightPos2 = vec3(cameraPos.x + 2, cameraPos.y, cameraPos.z - 2);
		vec3 lightPosArray[2] = { lightPos, lightPos2 };
		glUniform3fv(LightID, 2, (GLfloat*)lightPosArray);
	}
	glUseProgram(0);
}

vec3 cameraUp = -cross(vec3(0, 1, 0), vec3(0, 0, 1));

void moveCamera() {
	if (!cPress) return;

	if (glfwGetKey(window, GLFW_KEY_LEFT)) {
		theta -= radians(0.1f);
		prevX = 0.0 + sin(theta) * cos(phi) * radius;
		prevZ = 0.0 + cos(theta) * cos(phi) * radius;
		cameraPos = vec3(prevX, prevY, prevZ);
		gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), worldUp);
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
		theta += radians(0.1f);
		prevX = 0.0 + sin(theta) * cos(phi) * radius;
		prevZ = 0.0 + cos(theta) * cos(phi) * radius;
		cameraPos = vec3(prevX, prevY, prevZ);
		gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), worldUp);
	}

	if (glfwGetKey(window, GLFW_KEY_UP)) {
		phi += radians(0.1f);
		prevY = 0.0 + sin(phi) * radius;
		prevZ = 0.0 + cos(theta) * cos(phi) * radius;
		cameraPos = vec3(prevX, prevY, prevZ);
		gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), worldUp);
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN)) {
		phi -= radians(0.1f);
		prevY = 0.0 + sin(phi) * radius;
		prevZ = 0.0 + cos(theta) * cos(phi) * radius;
		cameraPos = vec3(prevX, prevY, prevZ);
		gViewMatrix = glm::lookAt(glm::vec3(prevX, prevY, prevZ), glm::vec3(0.0, 0.0, 0.0), worldUp);
	}
	updateLight();
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

	// For speed computation
	//double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		double deltaTime = currentTime - lastTime;
		if (deltaTime >= 1.0) { // If last prinf() was more than 1sec ago
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		if (cPress) {
			moveCamera();
		}

		// DRAWING POINTS
		renderScene(deltaTime);
		lastTime = currentTime;

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}