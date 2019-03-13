#include<vector>
#include<memory>
#include<glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
#include<map>
#include<iostream>

class Joint;

class Bone{
private:
	std::vector<std::shared_ptr<Joint>> joints;
	std::shared_ptr<Bone> parent;
	glm::quat q;
	glm::vec4 p;
	glm::quat bindq;
	glm::vec4 bindp;
	glm::mat4 E;
	glm::mat4 E0;
	std::string name;
	
public:
	Bone(std::string name, glm::quat _q, glm::vec4 _p);
	auto getE0(){return E0;}
	auto getE(){return E;}
	auto getQuat(){return q;}
	auto getPos(){return p;}
	void setQuatPos(glm::quat _q, glm::vec4 _p);
	void update();
	void setBindQuatPos(glm::quat _q, glm::vec4 _p);
	auto addJoint(std::shared_ptr<Joint> joint){joints.push_back(joint);}
	void updateJoints();
	void move(int axis, float amt){p[axis]+=amt;}
	void rotate(int axis, float amt);
	auto getName(){return name;}
	void reset();
};

class Joint{
private:
	std::pair<std::shared_ptr<Bone>, std::shared_ptr<Bone>> bones;
	// Offsets from parent and child. p1 = p_joint-p_parent, p2 = p_joint-p_child
	glm::vec4 p1, p2;
	glm::quat q;
	std::string name;
public:
	Joint(std::string name, std::shared_ptr<Bone> b1, std::shared_ptr<Bone> b2, glm::vec4 loc);
	auto getName(){return name;}
	auto getCName(){return name.c_str();}
	std::shared_ptr<Bone> getParent(){return bones.first;}
	std::shared_ptr<Bone> getChild(){return bones.second;}

	void rotate(int axis, float angle);
	// void move(int axis, float amt);
	void updateChild();
	glm::quat getQuat(){return q;}
	void setQuat(glm::quat _q);

	void reset();
};

class Vertex{
public:
	std::vector<float> weights;
	glm::vec4 p, p0;
	void setp(glm::vec4 _p){p = _p;}
};

