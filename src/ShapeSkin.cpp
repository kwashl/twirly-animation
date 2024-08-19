#include <iostream>
#include <fstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "ShapeSkin.h"
#include "GLSL.h"
#include "Program.h"

using namespace std;

Mode ShapeSkin::mode = Mode::None;
ShapeSkin::ShapeSkin() : prog(NULL),
						 elemBufID(0),
						 posBufID(0),
						 norBufID(0),
						 colBufID(0),
						 texBufID(0) {}

ShapeSkin::~ShapeSkin() {}

void ShapeSkin::loadMesh(const string &meshName)
{
	// Load geometry
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if (!rc)
	{
		cerr << errStr << endl;
	}
	else
	{
		posBuf = attrib.vertices;
		pos0Buf = vector<float>(attrib.vertices);
		vertices.resize(posBuf.size() / 3);
		for (auto i = 0; i < vertices.size(); i++)
		{
			vertices[i].p = glm::vec4(posBuf[3 * i], posBuf[3 * i + 1], posBuf[3 * i + 2], 1.0);
			vertices[i].p0 = vertices[i].p;
		}
		norBuf = attrib.normals;
		colBuf = vector<float>(norBuf);
		texBuf = attrib.texcoords;
		assert(posBuf.size() == norBuf.size());
		assert(pos0Buf.size() == norBuf.size());
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++)
		{
			// Loop over faces (polygons)
			const tinyobj::mesh_t &mesh = shapes[s].mesh;
			size_t index_offset = 0;
			for (size_t f = 0; f < mesh.num_face_vertices.size(); f++)
			{
				size_t fv = mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// access to vertex
					tinyobj::index_t idx = mesh.indices[index_offset + v];
					elemBuf.push_back(idx.vertex_index);
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void ShapeSkin::loadSkeleton(const std::string &fName)
{
	int nBones, nJoints, iRoot;
	ifstream in;
	in.open(fName);

	string line;
	getline(in, line);
	stringstream ss0(line); // nBones, iRoot
	ss0 >> nBones >> iRoot;
	bones.resize(nBones);
	map<std::string, shared_ptr<Bone> > boneMap;
	for (int i = 0; i < nBones; i++)
	{
		getline(in, line);
		stringstream ss(line); // bName, x, y, z
		string bName;
		ss >> bName;
		float x, y, z;
		ss >> x >> y >> z;
		glm::vec4 bloc = glm::vec4(x, y, z, 1.0);
		bones[i] = make_shared<Bone>(bName, glm::quat(1.0, 0, 0, 0), bloc);
		boneMap[bName] = bones[i];
	}
	setRoot(bones[iRoot]);
	root->update();

	getline(in, line); // nJoints
	stringstream ss1(line);
	ss1 >> nJoints;
	joints.resize(nJoints);
	for (int i = 0; i < nJoints; i++)
	{
		getline(in, line); // jName, parent, child, x, y, z
		stringstream ss(line);

		string jName, parent, child;
		ss >> jName >> parent >> child;

		float x, y, z;
		ss >> x >> y >> z;
		glm::vec4 jloc = glm::vec4(x, y, z, 1.0);

		// Create joint and add joint to parent bone
		joints[i] = make_shared<Joint>(jName, boneMap[parent], boneMap[child], jloc);
		boneMap[parent]->addJoint(joints[i]);
		printf("Added joint %s to %s\n", jName.c_str(), parent.c_str());
	}
}

void ShapeSkin::loadAttachment(const std::string &filename)
{
	int nverts, nbones;
	ifstream in;
	in.open(filename);
	if (!in.good())
	{
		cout << "Cannot read " << filename << endl;
		return;
	}

	string line;
	getline(in, line); // comment
	getline(in, line); // comment
	getline(in, line); // nverts, nbones
	stringstream ss0(line);
	ss0 >> nverts;
	ss0 >> nbones;
	assert(nverts == posBuf.size() / 3);
	int vert = 0;
	while (1)
	{
		getline(in, line); // Space separated weights
		// Parse until end of file.
		if (in.eof())
			break;

		stringstream ss(line);
		vertices[vert].weights.resize(nbones);

		// Load weights
		for (int ibone = 0; ibone < nbones; ibone++)
		{
			float wt;
			ss >> wt;
			vertices[vert].weights[ibone] = wt;
		}
		vert++;
	}
	in.close();
}

void ShapeSkin::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float),
				 posBuf.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &pos0BufID);
	glBindBuffer(GL_ARRAY_BUFFER, pos0BufID);
	glBufferData(GL_ARRAY_BUFFER, pos0Buf.size() * sizeof(float),
				 pos0Buf.data(), GL_STATIC_DRAW);

	// Send the normal array to the GPU
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float),
				 norBuf.data(), GL_STATIC_DRAW);

	// Vertex weights array
	glGenBuffers(1, &wBufID);
	glBindBuffer(GL_ARRAY_BUFFER, wBufID);
	glBufferData(GL_ARRAY_BUFFER, wBuf.size() * sizeof(float),
				 wBuf.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &colBufID);
	glBindBuffer(GL_ARRAY_BUFFER, colBufID);
	glBufferData(GL_ARRAY_BUFFER, colBuf.size() * sizeof(float),
				 colBuf.data(), GL_STATIC_DRAW);

	// No texture info
	texBufID = 0;

	// Send the element array to the GPU
	glGenBuffers(1, &elemBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elemBuf.size() * sizeof(unsigned int),
				 elemBuf.data(), GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Fill weight buffer if needed by GPU skinning.
	auto nb = getnBones();
	auto nw = getnw();
	wBuf.reserve(vertices.size() * nw);

	// Fill weights buffer.
	for (auto vertex : vertices)
	{
		for (int i = 0; i < nb; i++)
			wBuf.push_back(vertex.weights[i]);
		// To make stride a multiple of 4.
		for (int i = 0; i < nw - nb; i++)
			wBuf.push_back(0.0);
	}

	// Fill bone matrices.
	for (int i = 0; i < bones.size(); i++)
	{
		M0Buf[i] = bones[i]->getE0();
	}

	assert(glGetError() == GL_NO_ERROR);
}

// Bone weight to color
glm::vec3 getColorFromWeight(float w)
{
	glm::vec3 c0 = glm::vec3(0, 0, 0.5);
	glm::vec3 c1 = glm::vec3(1, 0, 0);
	return 2 * w * c1 + (1 - w) * c0;
}

void ShapeSkin::updateCPU()
{
	int iF = 0;
	for (int ibone = 0; ibone < bones.size(); ibone++)
	{
		auto Mj = bones[ibone]->getE();	  // animation local -> world matrix
		auto Mj0 = bones[ibone]->getE0(); // bind-pose local->world matrix
		auto Mj0i = glm::inverse(Mj0);	  // bind-pose world->local matrix

		MBuf[ibone] = Mj * Mj0i;
	}
	for (auto vertex : vertices)
	{
		glm::vec4 pos(0, 0, 0, 0); // initialize vec4 "pos" to zero
		for (int ibone = 0; ibone < bones.size(); ibone++)
		{
			// TODO Compute vertex position. Replace line below.
			//    vertex.weights[ibone]   holds the skinning weights
			// pos = vertex.p0; // default: don't do any transformations
			pos += vertex.weights[ibone] * MBuf[ibone] * vertex.p0; // CHANGED CODE

			// If bone selected, set color
			if (selBone == ibone)
			{
				glm::vec3 col = getColorFromWeight(vertex.weights[ibone]);
				colBuf[iF] = col[0];
				colBuf[iF + 1] = col[1];
				colBuf[iF + 2] = col[2];
			}
		}
		for (int i = 0; i < 3; i++)
		{
			posBuf[iF] = pos[i];
			// Color buffer not selected => Use normal buffer.
			if (selBone < 0 || selBone >= bones.size())
			{
				colBuf[iF] = norBuf[iF] * 0.5 + 0.5;
			}
			iF++;
		}
		vertex.setp(pos);
	}
	return;
}

void ShapeSkin::updateGPU()
{
	// TODO Update matrix buffer for GLSL
	// fill in MBuf[], which will later be passed to GPU shader as M[]

	for (int ibone = 0; ibone < bones.size(); ibone++)
	{
		auto Mj = bones[ibone]->getE();	  // animation local -> world matrix
		auto Mj0 = bones[ibone]->getE0(); // bind-pose local->world matrix
		auto Mj0i = glm::inverse(Mj0);

		MBuf[ibone] = Mj * Mj0i;
	}

	return;
}

void ShapeSkin::update()
{
	// Update the skeleton.
	if (bRootSet)
		root->update();

	if (bSkinGPU)
		updateGPU();
	else
		updateCPU();

	return;
}

void ShapeSkin::draw() const
{
	assert(prog);
	unsigned int h_pos, h_nor;
	unsigned int h_col;

	h_nor = prog->getAttribute("aNor");
	glEnableVertexAttribArray(h_nor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	h_pos = prog->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	if (bSkinGPU)
	{
		glBindBuffer(GL_ARRAY_BUFFER, pos0BufID);
		glBufferData(GL_ARRAY_BUFFER, pos0Buf.size() * sizeof(float), pos0Buf.data(), GL_STATIC_DRAW);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, posBufID);
		glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), posBuf.data(), GL_STATIC_DRAW);
	}
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	if (bSkinGPU)
	{
		// Uniforms
		// TODO Fill matrix uniforms
		glUniform1i(prog->getUniform("selBone"), selBone);
		glUniformMatrix4fv(prog->getUniform("M"), 18, GL_FALSE, glm::value_ptr(MBuf[0]));

		unsigned int h_w0, h_w1, h_w2, h_w3, h_w4;
		h_w0 = prog->getAttribute("w0");
		h_w1 = prog->getAttribute("w1");
		h_w2 = prog->getAttribute("w2");
		h_w3 = prog->getAttribute("w3");
		h_w4 = prog->getAttribute("w4");
		glEnableVertexAttribArray(h_w0);
		glEnableVertexAttribArray(h_w1);
		glEnableVertexAttribArray(h_w2);
		glEnableVertexAttribArray(h_w3);
		glEnableVertexAttribArray(h_w4);
		glBindBuffer(GL_ARRAY_BUFFER, wBufID);
		glBufferData(GL_ARRAY_BUFFER, wBuf.size() * sizeof(float), wBuf.data(), GL_STATIC_DRAW);
		unsigned stride = 20 * sizeof(float);
		glVertexAttribPointer(h_w0, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(0 * sizeof(float)));
		glVertexAttribPointer(h_w1, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(4 * sizeof(float)));
		glVertexAttribPointer(h_w2, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(8 * sizeof(float)));
		glVertexAttribPointer(h_w3, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(12 * sizeof(float)));
		glVertexAttribPointer(h_w4, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(16 * sizeof(float)));
	}
	else
	{
		// Color for weight when a bone is selected, otherwise just normal vectors.
		h_col = prog->getAttribute("aCol");
		glEnableVertexAttribArray(h_col);
		glBindBuffer(GL_ARRAY_BUFFER, colBufID);
		glBufferData(GL_ARRAY_BUFFER, colBuf.size() * sizeof(float), colBuf.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(h_col, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	// Draw triangles
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
	glDrawElements(GL_TRIANGLES, (int)elemBuf.size(), GL_UNSIGNED_INT, (const void *)0);

	// Disable vertex attributes
	glDisableVertexAttribArray(h_nor);
	glDisableVertexAttribArray(h_pos);
	if (bSkinGPU)
	{
		// TODO Disable any GPU skinning shader atttribute arrays
		//	  glDisableVertexAttribArray(h_w0);
		//	  glDisableVertexAttribArray(h_w1);
		//	  glDisableVertexAttribArray(h_w2);
		//	  glDisableVertexAttribArray(h_w3);
		//	  glDisableVertexAttribArray(h_w4);
	}
	else
	{
		glDisableVertexAttribArray(h_col);
	}

	// Unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ShapeSkin::increment(int amt)
{
	// Valid selected joint and axes
	if (selJoint >= 0 && axis >= 0 && axis < 3)
	{
		joints[selJoint]->rotate(axis, amt * 0.1);
		printf("Rotating axis %d for joint %s\n",
			   axis, joints[selJoint]->getCName());
	}
	else
	{
		root->rotate(axis, amt * 0.1);
	}
}

void ShapeSkin::move(int amt)
{
	root->move(axis, amt * 0.1);
}

void ShapeSkin::resetSelection()
{
	if (selJoint >= 0)
	{
		cout << "Resetting " << joints[selJoint]->getName() << " joint." << endl;
		joints[selJoint]->reset();
	}
	update();
}

void ShapeSkin::resetAll()
{
	cout << "Resetting all joints." << endl;
	for (auto joint : joints)
	{
		joint->reset();
	}
	if (bRootSet)
	{
		root->reset();
	}
}

void ShapeSkin::setSelBone(unsigned int _sb)
{
	selBone = _sb;
	selJoint = -1;
	printf("Selecting bone %d.\n", selBone);
}

void ShapeSkin::setSelJoint(unsigned int _sj)
{
	selJoint = _sj;
	selBone = -1;
	if (selJoint > getnJoints())
	{
		printf("Trying to select %d joint. Not available (max %zu).",
			   _sj, getnJoints());
		selJoint = -1;
	}
	else
	{
		// Can select joint.
		printf("Selecting joint %d.\n", selJoint);
	}
}

void ShapeSkin::recordJoints()
{
	frameQuats.push_back(vector<glm::quat>());

	// Root quat and pos
	auto q = root->getQuat();
	auto p = root->getPos();
	frameRootQuats.push_back(q);
	cout << q.w << " " << q.x << " " << q.y << " " << q.z << " ";
	frameRootPoses.push_back(p);
	cout << p.x << " " << p.y << " " << p.z << " ";

	// Joint quats
	for (auto joint : joints)
	{
		auto q = joint->getQuat();
		frameQuats.back().push_back(q);
		cout << q.w << " " << q.x << " " << q.y << " " << q.z << " ";
	}
	cout << endl;
	printf("Recording joints.\n");
}

void ShapeSkin::toggleGPU()
{
	bSkinGPU = !bSkinGPU;
	cout << "GPU mode = " << std::boolalpha << bSkinGPU << endl;
}

///////////////////////////////////////////////////////////
// CRinterpolate:   Catmull-Rom interpolation:  x(t) = T*M*G
///////////////////////////////////////////////////////////

float CRinterpolate(float t, glm::vec4 G)
{
	float Mh_vals[16] = {2, -3, 0, 1, -2, 3, 0, 0, 1, -2, 1, 0, 1, -1, 0, 0};
	glm::mat4 Mh = glm::make_mat4(Mh_vals);
	glm::vec4 Gh;
	glm::vec4 T(t * t * t, t * t, t, 1);
	Gh[0] = G[1];
	Gh[1] = G[2];
	Gh[2] = 0.5 * (G[2] - G[0]);
	Gh[3] = 0.5 * (G[3] - G[1]);
	auto A = Mh * Gh;
	return glm::dot(T, A);
}

///////////////////////////////////////////////////////////
// CRinterpolateV4:  Catmull-Rom interpolation across a set of 4D points
///////////////////////////////////////////////////////////

glm::vec4 CRinterpolateV4(float t, glm::vec4 &P, glm::vec4 P1, glm::vec4 P2, glm::vec4 P3, glm::vec4 P4)
{
	for (int d = 0; d < 4; d++)
	{ // for each dimension of vec4
		glm::vec4 G(P1[d], P2[d], P3[d], P4[d]);
		P[d] = CRinterpolate(t, G);
	}
}

///////////////////////////////////////////////////////////
// CRinterpolateQ:  Catmull-Rom interpolation across a set of quaternions
///////////////////////////////////////////////////////////

glm::quat CRinterpolateQ(float t, glm::quat &Q, glm::quat Q1, glm::quat Q2, glm::quat Q3, glm::quat Q4)
{
	for (int d = 0; d < 4; d++)
	{ // for each dimension of the quaternion
		glm::vec4 G(Q1[d], Q2[d], Q3[d], Q4[d]);
		Q[d] = CRinterpolate(t, G);
	}
}

///////////////////////////////////////////////////////////
// interpolate keyframes for skeleton
///////////////////////////////////////////////////////////

void ShapeSkin::interpolate(float aniTime)
{
	// Keyframes are placed at every integer time, i.e., aniTime = 0,1,2,3, ...
	// We identify keyframes N and N+1 such that   N < aniTime < N+1
	// and define    t = aniTime - N,  so that   0 <= t <= 1  for the local segment
	// Note:  aniTime += dt,  between frames, where    const float dt = 0.01   is defined in main.cpp (feel free to change)

	auto nKeyframes = frameQuats.size();
	if (aniTime >= (nKeyframes - 1))
		return;			   // aniTime beyond last keyframe?
	int N = (int)aniTime;  // previous keyframe
	float t = aniTime - N; // local time on segment;   0 <= t <= 1

	int N1 = max(N - 1, 0);
	int N2 = N;
	int N3 = N + 1;
	int N4 = min(N + 2, int(nKeyframes - 1));

	glm::vec4 rootposn;
	glm::quat rootq;
	CRinterpolateV4(t, rootposn, frameRootPoses[N1], frameRootPoses[N2], frameRootPoses[N3], frameRootPoses[N4]);
	CRinterpolateQ(t, rootq, frameRootQuats[N1], frameRootQuats[N2], frameRootQuats[N3], frameRootQuats[N4]);
	rootq = glm::normalize(rootq);
	//	cout << "aniTime=" << aniTime << " q=" << glm::to_string(rootq) << endl;

	root->setQuatPos(rootq, rootposn); // set the root position + orientation

	// Set joint quaternions:
	// note:  frameQuats is vector of keyframes, where each element is a vector of quaternions for all joints
	auto vq1 = frameQuats[N1];
	auto vq2 = frameQuats[N2];
	auto vq3 = frameQuats[N3];
	auto vq4 = frameQuats[N4];

	glm::quat q;
	for (int j = 0; j < joints.size(); j++)
	{ // for each joint
		CRinterpolateQ(t, q, frameQuats[N1][j], frameQuats[N2][j], frameQuats[N3][j], frameQuats[N4][j]);
		joints[j]->setQuat(glm::normalize(q)); // normalize, then set the current joint quaternion
	}
}

void ShapeSkin::loadAnim(const std::string &filename)
{
	ifstream in;
	in.open(filename);
	if (!in.good())
	{
		cout << "Cannot read " << filename << endl;
		return;
	}
	string line;

	// int frame=0;
	while (1)
	{
		getline(in, line);
		stringstream ss(line);
		if (in.eof())
			break;
		float w, x, y, z;
		ss >> w >> x >> y >> z;
		frameRootQuats.push_back(glm::quat(w, x, y, z));
		ss >> x >> y >> z;
		frameRootPoses.push_back(glm::vec4(x, y, z, 1.0));
		frameQuats.push_back(vector<glm::quat>());
		for (auto joint : joints)
		{
			ss >> w >> x >> y >> z;
			frameQuats.back().push_back(glm::quat(w, x, y, z));
		}
	}
	cout << "Loaded animation" << endl;
	in.close();
}
