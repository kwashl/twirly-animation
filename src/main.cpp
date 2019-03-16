#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <memory>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "ShapeSkin.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR    = "";    // Where the resources are loaded from
string MESH_FILE       = "";
string ATTACHMENT_FILE = "";
string FRAMES_FILE     = "";
string SKELETON_FILE   = "";
string ANIM_FILE       = "";
bool keyToggles[256] = {false};
unsigned int num = 0;

shared_ptr<Camera>    camera          = nullptr;
shared_ptr<ShapeSkin> shape           = nullptr;
shared_ptr<Program>   progSimple      = nullptr;
shared_ptr<Program>   progSkinCurrent = nullptr;
shared_ptr<Program>   progSkinGPU     = nullptr;
shared_ptr<Program>   progSkinCPU     = nullptr;

// Playback related variables
bool bPlayback = false;
float pbTime = 0;
const float dt = 0.01;

static void error_callback(int error, const char *description) {
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

void reset_num_mode(){
	num=0;
	ShapeSkin::mode = Mode::None;
}

static void char_callback(GLFWwindow *window, unsigned int key){
	keyToggles[key] = !keyToggles[key];
	switch(key) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		num = num*10+(key-'0');
		break;
	case 'f':
		shape->recordJoints();
		break;
	case '.':
		if(ShapeSkin::mode==Mode::Joints) shape->setSelJoint(num);
		if(ShapeSkin::mode==Mode::Bones ) shape->setSelBone (num);
		if(ShapeSkin::mode==Mode::Axes  ) shape->setSelAxis (num);
		reset_num_mode();
		break;

/* Handling modes. */
	case 'n':
		reset_num_mode();
		shape->clearSel();
		break;
	case 'b':
		ShapeSkin::mode = Mode::Bones;
		printf("Bone selection mode.");
		break;
	case 'j':
		ShapeSkin::mode = Mode::Joints;
		log_reg("Joint selection mode.");
		break;
	case 'a':
		ShapeSkin::mode = Mode::Axes;
		log_reg("Axis selection mode.");
		break;
	case 'p':
		bPlayback = true;
		log_reg("Starting playback.");
		break;
	case 'g':
		shape->toggleGPU();
		progSkinCurrent = shape->useGPU()? progSkinGPU: progSkinCPU;
		log_err("GPU skinning not yet implemented.\n"
              "If attributes are not used in vertex shader, the program may crash.\n"
              "-----------------------------\n");
		break;
	case '+':
		shape->increment(1);
		break;
	case '-':
		shape->increment(-1);
		break;
	case '=':
		shape->resetSelection();
		break;
	case 'r':
		shape->resetAll();
		break;
	case '>':
		shape->move(1);
		break;
	case '<':
		shape->move(-1);
		break;
	}
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse) {
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved(xmouse, ymouse);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = mods & GLFW_MOD_SHIFT;
		bool ctrl  = mods & GLFW_MOD_CONTROL;
		bool alt   = mods & GLFW_MOD_ALT;
		camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
	}
}

// Animation file (animFile) is optional.
void loadScene(const string &meshFile, const string &attachmentFile, const string &skelFile, 
					const string &animFile="") {
	keyToggles[(unsigned)'c'] = true;
	
	camera = make_shared<Camera>();
	
	shape = make_shared<ShapeSkin>();
	shape->loadMesh(meshFile);
	shape->loadAttachment(attachmentFile);
	shape->loadSkeleton(skelFile);
	// Load animation if available.
	if(!animFile.empty()) shape->loadAnim(animFile);
	
	// For drawing the grid, etc.
	progSimple = make_shared<Program>();
	progSimple->setShaderNames(RESOURCE_DIR + "simple_vert.glsl", RESOURCE_DIR + "simple_frag.glsl");
	progSimple->setVerbose(true);
	
	// For skinned shape, CPU/GPU
	// TODO Write GPU skinning shader and associated code and then uncomment these.
	progSkinGPU = make_shared<Program>();
	progSkinGPU->setShaderNames(RESOURCE_DIR + "skin_lin_vert.glsl",
	                           RESOURCE_DIR + "skin_lin_frag.glsl");

	progSkinCPU = make_shared<Program>();

	progSkinCPU->setShaderNames(RESOURCE_DIR + "skin_vert.glsl",
										RESOURCE_DIR + "skin_frag.glsl");
	progSkinCurrent = shape->useGPU()?progSkinGPU:progSkinCPU;
}

void addAndCheckAttr(shared_ptr<Program> prog, string sAttr){
	if(!progSkinGPU->addAttribute(sAttr)) 
      log_err("Could not add attribute %s. Attribute may be unused in shader.", sAttr.c_str());
}

void init() {
	// Non-OpenGL things
	loadScene(MESH_FILE, ATTACHMENT_FILE, SKELETON_FILE, ANIM_FILE);
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	shape->init();
	progSimple->init();
	progSimple->addUniform("P");
	progSimple->addUniform("MV");

	progSkinGPU->init();
	progSkinGPU->addUniform("selBone");
	progSkinGPU->addAttribute("aPos");
	progSkinGPU->addAttribute("aNor");
   addAndCheckAttr(progSkinGPU, "w0");
   addAndCheckAttr(progSkinGPU, "w1");
   addAndCheckAttr(progSkinGPU, "w2");
   addAndCheckAttr(progSkinGPU, "w3");
   addAndCheckAttr(progSkinGPU, "w4");
	progSkinGPU->addUniform("P");
	progSkinGPU->addUniform("MV");

	progSkinCPU->init();
	progSkinCPU->addAttribute("aCol");
	progSkinCPU->addAttribute("aPos");
	progSkinCPU->addAttribute("aNor");
	progSkinCPU->addUniform("P");
	progSkinCPU->addUniform("MV");
	
	// Initialize time.
	glfwSetTime(0.0);
	
	GLSL::checkError(GET_FILE_LINE);
}

void render() {
	// GLFW time may be used to time animation.
	// double t = glfwGetTime();

	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	// Use the window size for camera.
	glfwGetWindowSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);
	GLSL::checkError(GET_FILE_LINE);
	
	// Draw grid
	progSimple->bind();
	glUniformMatrix4fv(progSimple->getUniform("P") , 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progSimple->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	float gridSizeHalf = 5.0f;
	int gridNx = 11;
	int gridNz = 11;
	glLineWidth(1);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_LINES);
	for(int i = 0; i < gridNx; ++i) {
		float alpha = i / (gridNx - 1.0f);
		float x = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
		glVertex3f(x, 0, -gridSizeHalf);
		glVertex3f(x, 0,  gridSizeHalf);
	}
	for(int i = 0; i < gridNz; ++i) {
		float alpha = i / (gridNz - 1.0f);
		float z = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
		glVertex3f(-gridSizeHalf, 0, z);
		glVertex3f( gridSizeHalf, 0, z);
	}

	glEnd();
	progSimple->unbind();
  
	// Interpolation for playback.
	if(bPlayback){
		if(pbTime<=shape->totalTime()){ 
			shape->interpolate(pbTime);
			pbTime+=dt;
		}
		else{
			bPlayback = false;
			pbTime = 0;
			log_reg("Playback done.\n");
		}
		// printf("Playback time %3.2f\n", pbTime);
	}

	// Updating every time we render is not ideal in the case of CPU skinning, since vertex positions
	// do not change at every timestep. If the vertex positions are responsibly updated after every
	// relevant change, such as joint rotation, this update may be moved out of here.
	shape->update();

	// Draw character
	MV->pushMatrix();
	progSkinCurrent->bind();
	glUniformMatrix4fv(progSkinCurrent->getUniform("P") , 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(progSkinCurrent->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	shape->setProgram(progSkinCurrent);
	shape->draw();
	progSkinCurrent->unbind();
	GLSL::checkError(GET_FILE_LINE);

	MV->popMatrix();

	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();

	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv) {
	if(argc < 5) {
		cout << "Usage: A4 <SHADER DIR> <MESH FILE> <ATTACHMENT FILE> <SKELETON FILE> (<ANIM FILE>)"<<endl
			  <<"<ANIM FILE> is optional. If not provided, no animation is loaded." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	MESH_FILE = argv[2];
	ATTACHMENT_FILE = argv[3];
	SKELETON_FILE = argv[4];
	if(argc==6){
		ANIM_FILE = argv[5];
	}
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
