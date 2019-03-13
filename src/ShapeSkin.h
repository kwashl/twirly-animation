#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Hierarchy.h"

class MatrixStack;
class Program;

enum class Mode{Bones, Joints, Axes, None};

class ShapeSkin{
public:
	ShapeSkin();
	virtual ~ShapeSkin();

	// Loading OBJ file.
	void loadMesh(const std::string &meshName);
	// Load bones and joints info.
	void loadSkeleton(const std::string &fname);
	// Load bone attachment weights for vertices.
	void loadAttachment(const std::string &filename);
	// Load animation if provided.
	void loadAnim(const std::string &filename);
	
	void setProgram(std::shared_ptr<Program> p) { prog = p; }
	void init();
	void draw() const;
	void update();

	void updateCPU();
	void updateGPU();

	void increment(int amt);
	void setSelJoint(unsigned int _sj);
	void setSelBone (unsigned int _sb);
	void setSelAxis (unsigned int  _a){axis = _a; std::cout<<"Selected axis "<<_a<<std::endl;}
	void clearSel   (){selJoint = -1; selBone = -1; axis = 0; mode = Mode::None;}
	void recordJoints();
	void playback();
	static Mode mode;
	void interpolate(float t);
	float totalTime(){return frameQuats.size()-1;}
	void setRoot(std::shared_ptr<Bone> _root){root = _root; bRootSet = true;}
	void resetSelection();
	void resetAll();
	void toggleGPU();
	auto useGPU(){return bSkinGPU;}
	void move(int amt);
	
private:
	bool bSkinGPU = false;
	std::shared_ptr<Program> prog;

	// Root of the hierarchy. No parent bone.
	std::shared_ptr<Bone> root;
	bool bRootSet = false;

	// Buffers to use for sharing data with shaders
	std::vector<unsigned int> elemBuf;
	std::vector<float> posBuf, pos0Buf;
	glm::mat4 MBuf[18];
	glm::mat4 M0Buf[18];
	std::vector<float> norBuf;
	std::vector<float> wBuf;
	std::vector<float> colBuf;
	std::vector<float> texBuf;

	// Vertex weights
	std::vector<Vertex> vertices;
	// Actual number of bones
	auto getnBones(){return bones.size();};
	auto getnJoints(){return joints.size();};
	// Multiple of 4;
	int getnw(){return 20;};

	// Skeleton
	std::vector<std::shared_ptr<Bone>> bones;
	std::vector<std::shared_ptr<Joint>> joints;
	
	//// Playback variables
	// One quat per joint per frame
	std::vector<std::vector<glm::quat>> frameQuats;
	// One quat per frame specifying rotation of skeleton root
	std::vector<glm::quat> frameRootQuats;
	// One quat per frame specifying translation of skeleton root
	std::vector<glm::vec4> frameRootPoses;
	unsigned wBufID, MBufID, M0BufID, pos0BufID;
	unsigned axesIntBufID;
	unsigned axesPosBufID;
	unsigned axesElemBufID;
	unsigned elemBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned colBufID;
	unsigned texBufID;
	int selJoint = -1;
	int selBone  = -1;
	int axis = 0;
};
