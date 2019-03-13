#include "Hierarchy.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include<glm/gtx/quaternion.hpp>
#include<functional>
#include<algorithm>
#include<iostream>

using namespace std;

Joint::Joint(std::string _name, std::shared_ptr<Bone> _p, std::shared_ptr<Bone> _c, glm::vec4 loc)
	: name(_name){
	bones = make_pair(_p, _c);
	q = glm::quat(1,0,0,0);
	p1 = loc-_p->getPos();
	p2 = loc-_c->getPos();
}
	
void Joint::updateChild(){
	auto parent = getParent();
	auto parentGlobalQuat = parent->getQuat();
	
	// Joint location
	auto jointGlobalPos = glm::rotate(parentGlobalQuat, p1) + parent->getPos();
	
	// Child orientation
	auto childGlobalQuat = glm::cross(parentGlobalQuat, q);
	
	// Child location
	auto childGlobalPos = glm::rotate(childGlobalQuat, -p2) + jointGlobalPos;
	
	// Update
	getChild()->setQuatPos(childGlobalQuat, childGlobalPos);
	getChild()->update();

	return;
}

void Joint::rotate(int axis, float angle){
	glm::vec3 v(0,0,0);
	v[axis] = 1;
	q = glm::rotate(q, angle, v);
	updateChild();
	return;
}

void Bone::rotate(int axis, float angle){
	glm::vec3 v(0,0,0);
	v[axis] = 1;
	q = glm::rotate(q, angle, v);
	updateJoints();
	return;
}

void Joint::setQuat(glm::quat _q){
	// std::cout<<"Setting joint "<<name<<" "<<
	//    q.x<<" "<<q.y<<" "<<q.z<<" "<<q.w<<std::endl;
	q = _q; 
}

void Joint::reset(){
	setQuat(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
	updateChild();
}

Bone::Bone(std::string _name, glm::quat _q, glm::vec4 _p){
	name = _name;
	setBindQuatPos(_q, _p);
}

void Bone::setBindQuatPos(glm::quat _q, glm::vec4 _p){
	q = _q; p = _p;
	bindp = _p; bindq = _q;
	E0 = glm::mat4_cast(_q);
	E0[3] = _p;
	E = E0;
	return;
}

void Bone::reset(){
	q = bindq; p = bindp;
}

void Bone::setQuatPos(glm::quat _q, glm::vec4 _p){
	q = _q; p = _p; return;
}

void Bone::update(){
	// cout<<"Updating bone "<<name<<endl;
	E = glm::mat4_cast(q);
	E[3] = p;
	// cout<<glm::to_string(E)<<endl;
	updateJoints();
	return;
}

void Bone::updateJoints(){
	for_each(joints.begin(), joints.end(), mem_fn(&Joint::updateChild));
	return;
}
