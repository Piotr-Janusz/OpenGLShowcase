#pragma once
#include <glm/gtx/matrix_decompose.hpp>

class ModelClass
{
public:
	virtual void Model(glm::mat4* model) = 0;
	virtual void Update() = 0;
	virtual vector<Object> ParseObj() = 0;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	vector<Object> currentObjs;
	int looker = 0;
	glm::vec3 nextPos;
	glm::vec3 up;
	float yaw;
	float pitch;
};




vector<triangle> generateSphere2(int slices, int stacks)
{
	// slices is latitude
	// stacks is longitude
	vector<vertex> verticies;
	vector<triangle> tris;
	float d1 = M_PI / float(slices);
	float d2 = 2 * M_PI / float(stacks);
	float da1;
	float da2;

	for (int i = 0; i <= slices; ++i)
	{
		da1 = M_PI / 2 - 1 * d1;
		float xy = std::cosf(da1);
		float z = std::sinf(da1);

		for (int w = 0; w <= stacks; ++w)
		{
			da2 = w * d2;
			vertex v1;
			v1.vc.x = xy * std::cosf(da2);
			v1.vc.y = xy * std::sinf(da2);
			v1.vc.z = z;
			v1.tc.x = (float)w / stacks;
			v1.tc.y = (float)i / slices;
			verticies.push_back(v1);
		}
	}

	for (int i = 0; i < slices; ++i)
	{
		int index1 = i * (stacks + 1);
		int index2 = index1 + stacks + 1;
		for (int w = 0; w < stacks; ++w, ++index1, ++index2)
		{
			if (i != 0)
			{
				triangle t1(verticies[index1], verticies[index2], verticies[index1 + 1]);
				tris.push_back(t1);
				std::cout << t1.verts->nc.x << " " << t1.verts->nc.y << " " << t1.verts->nc.z << std::endl;
			}

			if (i != (slices - 1))
			{
				triangle t1(verticies[index1 + 1], verticies[index2], verticies[index2 + 1]);
				tris.push_back(t1);
				std::cout << t1.verts->nc.x << " " << t1.verts->nc.y << " " << t1.verts->nc.z << std::endl;
			}
		}
	}
	return tris;
}


vector<triangle> generateSphere3(int slices, int stacks)
{
	// slices is latitude
	// stacks is longitude
	vector<vertex> verticies;
	vector<triangle> tris;
	vertex v0;
	v0.vc = vec3(0, 1, 0);
	v0.tc = vec2(0, 0);
	verticies.push_back(v0);


	for (int i = 0; i <= stacks - 1; i++)
	{
		float phi = M_PI * double(i + 1) / double(stacks);
		for (int j = 0; j < slices; j++)
		{
			float theta = 2.0 * M_PI * double(j) / double(slices);
			float x = std::sin(phi) * std::cos(theta);
			float y = std::cos(phi);
			float z = std::sin(phi) * std::sin(theta);
			vertex vf;
			vf.vc = vec3(x, y, z);
			vf.tc = vec2(0, 0);
			verticies.push_back(vf);
		}
	}

	vertex v1;
	v1.vc = vec3(0, -1, 0);
	verticies.push_back(v1);

	for (int i = 0; i < slices; ++i)
	{
		int i0 = i + 1;
		int i1 = (i + 1) % slices + 1;
		triangle t1(v0, verticies[i1], verticies[i0]);
		tris.push_back(t1);
		i0 = i + slices * (stacks - 2) + 1;
		i1 = (i + 1) % slices + slices * (stacks - 2) + 1;
		triangle t2(v1, verticies[i0], verticies[i1]);
		tris.push_back(t2);
	}

	for (int j = 0; j < stacks - 2; j++)
	{
		int j0 = j * slices + 1;
		int j1 = (j + 1) * slices + 1;
		for (int i = 0; i < slices; i++)
		{
			int i0 = j0 + 1;
			int i1 = j0 + (i + 1) % slices;
			int i2 = j1 + (i + 1) % slices;
			int i3 = j1 + i;
			triangle t1(verticies[i0], verticies[i2], verticies[i1]);
			triangle t2(verticies[i0], verticies[i2], verticies[i3]);
			tris.push_back(t1);
			tris.push_back(t2);
		}
	}

	// find normals for everything

	for (int i = 0; i < tris.size(); i++)
	{
		triangle current = tris[i];
		vec3 p1 = current.verts[0].vc;
		vec3 p2 = current.verts[1].vc;
		vec3 p3 = current.verts[2].vc;
		vec3 A(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
		vec3 B(p3.x - p1.x, p3.y - p1.y, p3.z - p2.z);
		float nx = A.y * B.z - A.z * B.y;
		float ny = A.z * B.x - A.x * B.z;
		float nz = A.x * B.y - A.y * B.x;
		current.verts[0].nc = vec3(nx, ny, nz);
		current.verts[1].nc = vec3(nx, ny, nz);
		current.verts[2].nc = vec3(nx, ny, nz);
	}

	return tris;
}

vector<triangle> buildSphere(int sectors, int stacks)
{
	float x, y, z, xy;
	float nx, ny, nz;
	float u, v;

	float sectorStep = 2 * M_PI / sectors;
	float stackStep = M_PI / stacks;
	float sectorAngle, stackAngle;

	vector<vertex> verts;
	vector<triangle> tris;

	for(int i = 0; i <= stacks; ++i)
	{
		stackAngle = M_PI / 2 - i * stackStep;
		xy = cosf(stackAngle);
		z = sinf(stackAngle);

		for (int j = 0; j <= sectors; ++j)
		{
			sectorAngle = j * sectorStep;

			x = xy * cosf(sectorAngle);
			y = xy * sinf(sectorAngle);
			vec3 pos(x, y, z);

			nx = x;
			ny = y;
			nz = z;
			vec3 nor(nx, ny, nz);

			u = (float)j / sectors;
			v = (float)i / stacks;
			vec2 tex(u, v);

			verts.push_back(vertex(pos, tex, nor));
		}
	}

	// Build triangles from the verticies

	for (int i = 0; i < stacks; ++i)
	{
		int k1 = i * (sectors + 1);
		int k2 = k1 + sectors + 1;
		for (int j = 0; j < sectors; j++, ++k1, ++k2)
		{
			if (i != 0)
			{
				triangle temp(verts[k1], verts[k2], verts[k1 + 1]);
				tris.push_back(temp);
			}

			if (i != (stacks - 1))
			{
				triangle temp(verts[k1 + 1], verts[k2], verts[k2 + 1]);
				tris.push_back(temp);
			}
		}
	}

	return tris;
}

glm::vec2 constantVector(int v)
{
	int h = v % 3;
	if (h == 0)
	{
		return glm::vec2(1.0f, 1.0f);
	}
	if (h == 1)
	{
		return glm::vec2(-1.0f, 1.0f);
	}
	if (h == 2)
	{
		return glm::vec2(-1.0f, -1.0f);
	}
	else
	{
		return glm::vec2(1.0f, -1.0f);
	}
}

float Lerp(float  t, float a1, float a2)
{
	return a1 + t * (a2 - a1);
}


float Noise2D(float x, float y)
{
	int X = ((int) floor(x)) % 255;
	int Y = ((int) floor(y)) % 255;
	float xf = x - floor(x);
	float yf = y - floor(y);

	glm::vec2 topRight = glm::vec2(xf - 1.0f, yf - 1.0f);
	glm::vec2 topLeft = glm::vec2(xf, yf - 1.0f);
	glm::vec2 bottomRight = glm::vec2(xf - 1.0f, yf);
	glm::vec2 bottomLeft = glm::vec2(xf, yf);

	// Create the permutation table

	vector<int> permutation;
	for (int i = 0; i < 256; i++)
	{
		permutation.push_back(i);
	}

	// Shuffle it

	for (int i = 0; i < 256; i++)
	{
		int index = rand() % 255;
		int temp = permutation[index];

		permutation[index] = permutation[i];
		permutation[i] = temp;
	}

	// Double it for looping
	for (int i = 0; i < 256; i++)
	{
		permutation.push_back(permutation[i]);
	}

	int valueTopRight = permutation[permutation[X + 1] + Y + 1];
	int valueTopLeft = permutation[permutation[X] + Y + 1];
	int valueBottomRight = permutation[permutation[X + 1] + Y];
	int valueBottomLeft = permutation[permutation[X] + Y];

	float dotTopRight = glm::dot(topRight, constantVector(valueTopRight));
	float dotTopLeft = glm::dot(topLeft, constantVector(valueTopLeft));
	float dotBottomRight = glm::dot(bottomRight, constantVector(valueBottomRight));
	float dotBottomLeft = glm::dot(bottomLeft, constantVector(valueBottomLeft));

	float u = ((6 * xf - 15) * xf + 10) * xf * xf * xf;
	float v = ((6 * yf - 15) * yf + 10) * yf * yf * yf;

	return Lerp(u, Lerp(v, dotBottomLeft, dotTopLeft), Lerp(v, dotBottomRight, dotTopRight));
}

float FractalBrownianMotion(int x, int y, int numOctaves)
{
	float result = 0.0f;
	float amplitude = 1.0f;
	float frequancy = 0.005f;

	for (int o = 0; o < numOctaves; o++)
	{
		float n = amplitude * Noise2D(x * frequancy, y * frequancy);
		result += n;

		amplitude *= 0.5;
		frequancy *= 2.0;
	}

	return result;
}

vector<triangle> createBumpyPlane(int xLen, int yLen)
{
	vector<vertex> vecs;
	vector<triangle> tris;
	vector<triangle> faceNormals;
	for (int y = 0; y < yLen; y++)
	{
		for (int x = 0; x < xLen; x++)
		{
			vec3 pos(x,(FractalBrownianMotion(x, y, 4) * 1), y);
			vec2 tex((float) x / (float) xLen, (float) y / (float) yLen);
			vec3 nor(0, 0, 0);
			vecs.push_back(vertex(pos, tex, nor));
		}
	}

	// Build tris

	for (int y = 0; y < yLen - 1; y++)
	{
		for (int x = 0; x < xLen - 1; x++)
		{
			int index = x + (y * xLen);
			if (index % xLen == 0)
			{
				continue;
			}
			//std::cout << index << std::endl;
			tris.push_back(triangle(vecs[index], vecs[index + 1], vecs[index + xLen]));
			tris.push_back(triangle(vecs[index + 1], vecs[index + xLen], vecs[index + xLen + 1]));
		}
	}



	for (int i = 0; i < tris.size(); i++)
	{
		vertex a = tris[i].verts[0];
		vertex b = tris[i].verts[1];
		vertex c = tris[i].verts[2];
		glm::vec3 bminusa = glm::vec3(b.vc.x - a.vc.x, b.vc.y - a.vc.y, b.vc.z - a.vc.z);
		glm::vec3 cminusa = glm::vec3(c.vc.x - a.vc.x, c.vc.y - a.vc.y, c.vc.z - a.vc.z);
		//glm::vec3 normal = glm::normalize(glm::cross(bminusa, cminusa));
		//tris[i].verts[0].nc.x += normal.x;
		//tris[i].verts[0].nc.y += normal.y;
		//tris[i].verts[0].nc.z += normal.z;

		//tris[i].verts[1].nc.x += normal.x;
		//tris[i].verts[1].nc.y += normal.y;
		//tris[i].verts[1].nc.z += normal.z;

		//tris[i].verts[2].nc.x += normal.x;
		//tris[i].verts[2].nc.y += normal.y;
		//tris[i].verts[2].nc.z += normal.z;

		//glm::vec3 p = glm::cross(bminusa, cminusa);
		glm::vec3 p;
		p.x = (bminusa.y * cminusa.z) - (bminusa.z * cminusa.y);
		p.y = (bminusa.z * cminusa.x) - (bminusa.x * cminusa.z);
		p.z = (bminusa.x * cminusa.y) - (bminusa.y * cminusa.x);
		tris[i].verts[0].nc.x += p.x;
		tris[i].verts[0].nc.y += p.y;
		tris[i].verts[0].nc.z += p.z;

		tris[i].verts[1].nc.x += p.x;
		tris[i].verts[1].nc.y += p.y;
		tris[i].verts[1].nc.z += p.z;

		tris[i].verts[2].nc.x += p.x;
		tris[i].verts[2].nc.y += p.y;
		tris[i].verts[2].nc.z += p.z;
	}

	for (int i = 0; i < tris.size(); i++)
	{
		for (int v = 0; v < 3; v++)
		{
			vertex current = tris[i].verts[v];
			float currentX = current.nc.x;
			float currentY = current.nc.y;
			float currentZ = current.nc.z;
			glm::vec3 normalized = glm::normalize(glm::vec3(current.nc.x, current.nc.y, current.nc.z));
			//std::cout << currentX <<" " << currentY << " " << currentZ << std::endl;
			tris[i].verts[v].nc.x = normalized.x;
			tris[i].verts[v].nc.y = normalized.y;
			tris[i].verts[v].nc.z = normalized.z;
		}
	}
	//std::cout << "Here" << std::endl;

	for (int i = 0; i < tris.size(); i++)
	{
		if (i % 2 == 0)
		{
			for (int v = 0; v < 3; v++)
			{
				tris[i].verts[v].nc.x = -tris[i].verts[v].nc.x;
				tris[i].verts[v].nc.y = -tris[i].verts[v].nc.y;
				tris[i].verts[v].nc.z = -tris[i].verts[v].nc.z;
			}
		}
		
	}
	return tris;
}

class BirdQuad : public ModelClass
{
public:
	BirdQuad()
	{
		position = glm::vec3(0.f, 1.f, 0.f);
		rotation = glm::vec3(0.f, 0.f, 0.f);
		scale = glm::vec3(1.f, 1.f, 1.f);
	}
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), position);
		(*model) = glm::rotate((*model), rotation.x, glm::vec3(1.f, 0.f, 0.f));
		(*model) = glm::rotate((*model), rotation.y, glm::vec3(0.f, 1.f, 0.f));
		(*model) = glm::rotate((*model), rotation.z, glm::vec3(0.f, 0.f, 1.f));
		(*model) = glm::scale((*model), scale);
	}
	void Update()
	{
		rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/bird/textured_quad.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};

class Building : public ModelClass
{
public:
	Building()
	{
		position = glm::vec3(0.f, 0.f, 0.f);
		rotation = glm::vec3(0.f, 0.f, 0.f);
		scale = glm::vec3(0.25f, 0.25f, 0.25f);
	}
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), position);
		(*model) = glm::rotate((*model), rotation.x, glm::vec3(1.f, 0.f, 0.f));
		(*model) = glm::rotate((*model), rotation.y, glm::vec3(0.f, 1.f, 0.f));
		(*model) = glm::rotate((*model), rotation.z, glm::vec3(0.f, 0.f, 1.f));
		(*model) = glm::scale((*model), scale);
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/backrooms/untitled.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};

class Table : public ModelClass
{
public:
	Table()
	{
		position = glm::vec3(0.f, 0.5f, -0.9f);
		rotation = glm::vec3(0.f, 0.f, 0.f);
		scale = glm::vec3(2.f, 2.f, 2.f);
	}
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), position);
		(*model) = glm::rotate((*model), rotation.x, glm::vec3(1.f, 0.f, 0.f));
		(*model) = glm::rotate((*model), rotation.y, glm::vec3(0.f, 1.f, 0.f));
		(*model) = glm::rotate((*model), rotation.z, glm::vec3(0.f, 0.f, 1.f));
		(*model) = glm::scale((*model), scale);
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/table/table.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};

class TreeTest : public ModelClass
{
public:
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(0.f, -1.8f, -3.f));
		(*model) = glm::rotate((*model), (float)glfwGetTime() / 2, glm::vec3(0.f, 1.f, 0.f));
		(*model) = glm::scale((*model), glm::vec3(.005f, .005f, .005f));
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/white_oak/white_oak.obj", &objs);
		return objs;
	}
};

class Sphere : public ModelClass
{
public:
	Sphere()
	{
		position = glm::vec3(-2.75f, 1.35f, -1.45f);
	}
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::rotate((*model), (float)glfwGetTime() / 2, glm::vec3(0.f, 1.f, 0.f));
		(*model) = glm::scale((*model), glm::vec3(0.25f, 0.25f, 0.25f));
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		Object o1;
		o1.tris = buildSphere(20, 20);
		strcpy(o1.mtl.fil_name, "objs/bird/bird.jpg");
		objs.push_back(o1);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		
		return objs;
	}
};

class BumpyPlane : public ModelClass
{
public:
	BumpyPlane()
	{
		position = glm::vec3(-60.f, -3.f, -60.f);
	}
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::scale((*model), glm::vec3(0.5f, 0.5f, 0.5f));
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		Object o1;
		o1.tris = createBumpyPlane(200,200);
		strcpy(o1.mtl.fil_name, "objs/tex_Water.jpg");
		objs.push_back(o1);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));

		return objs;
	}
};

class Ground : public ModelClass
{
public:
	Ground()
	{
		position = glm::vec3(-60.f, -7.f, -60.f);
	}
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::scale((*model), glm::vec3(1.f, 1.f, 1.f));
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		Object o1;
		o1.tris = createBumpyPlane(100, 100);
		strcpy(o1.mtl.fil_name, "objs/tex_Dirt.jpg");
		objs.push_back(o1);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));

		return objs;
	}
};

class FakeSphere : public ModelClass
{
public:
	FakeSphere()
	{
		position = glm::vec3(0.f, -2.f, 0.f);
	}
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::rotate((*model), (float)glfwGetTime() / 2, glm::vec3(0.f, 1.f, 0.f));
		(*model) = glm::scale((*model), glm::vec3(2.0f, 2.0f, 2.0f));
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/sphere/untitled.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};

class Carrier : public ModelClass
{
public:
	Carrier()
	{
		position = glm::vec3(0.f, -2.f, 0.f);
	}
	void Model(glm::mat4* model)
	{
		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::scale((*model), glm::vec3(3.0f, 3.0f, 3.0f));
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/carrier/carrier.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};

class Jet : public ModelClass
{
public:
	Jet()
	{
		position = glm::vec3(0.f, -1.f, 0.f);
		rotation = glm::vec3(0.f, 0.f, 0.f);
		looker = 1;
	}
	void Model(glm::mat4* model)
	{
		glm::mat4 test = glm::inverse(glm::lookAt(position, position + nextPos, glm::vec3(0, 1, 0)));
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(test, scale, rotation, translation, skew, perspective);
		glm::vec3 rotationEuler = glm::eulerAngles(rotation);
		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::rotate((*model), rotationEuler.x, glm::vec3(1.f, 0.f, 0.f));
		(*model) = glm::rotate((*model), rotationEuler.y, glm::vec3(0.f, 1.f, 0.f));
		(*model) = glm::rotate((*model), rotationEuler.z, glm::vec3(0.f, 0.f, 1.f));
		(*model) = glm::rotate((*model), glm::radians(180.0f), glm::vec3(0.f, 1.f, 0.f));

		(*model) = glm::scale((*model), glm::vec3(0.4f, 0.4f, 0.4f));

	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/plane/untitled.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};

class Jet2 : public ModelClass
{
public:
	Jet2()
	{
		position = glm::vec3(0.f, -1.f, 0.f);
		rotation = glm::vec3(0.f, 0.f, 0.f);
	}
	void Model(glm::mat4* model)
	{
		glm::mat4 test = glm::inverse(glm::lookAt(position, position + nextPos, up));
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(test, scale, rotation, translation, skew, perspective);
		glm::vec3 rotationEuler = glm::eulerAngles(rotation);
		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::scale((*model), glm::vec3(0.4f, 0.4f, 0.4f));
		(*model) = glm::rotate((*model), -glm::radians(yaw), glm::vec3(0.f, 1.f, 0.f));
		if (yaw >= 90 && yaw <= 270)
		{
			(*model) = glm::rotate((*model), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
		}
		(*model) = glm::rotate((*model), -rotationEuler.z, glm::vec3(0.f, 0.f, 1.f));
		(*model) = glm::rotate((*model), rotationEuler.x, glm::vec3(1.f, 0.f, 0.f));
		

		
		(*model) = glm::rotate((*model), glm::radians(180.0f), glm::vec3(0.f, 1.f, 0.f));
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/plane/untitled.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};


class Buoy : public ModelClass
{
public:
	Buoy()
	{
		position = glm::vec3(17.8793, -2.12803, -1.88456);
	}
	void Model(glm::mat4* model)
	{

		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::translate((*model), glm::vec3(0, glm::sin(((glfwGetTime() + (position.x * 2)))) / 2.4, 0));
		(*model) = glm::scale((*model), glm::vec3(0.2f, 0.2f, 0.2f));
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/buoy/untitled.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};

class Border : public ModelClass
{
public:
	Border()
	{
		position = glm::vec3(-10,-10,-11);
	}
	void Model(glm::mat4* model)
	{

		(*model) = glm::translate((*model), glm::vec3(position));
		(*model) = glm::scale((*model), glm::vec3(5.f, 10.f, 5.f));
	}
	void Update()
	{
		//rotation.y = (float)glfwGetTime() / 2;
	}
	virtual vector<Object> ParseObj()
	{
		vector<Object> objs;
		obj_parse("objs/border/border.obj", &objs);
		std::copy(objs.begin(), objs.end(), back_inserter(currentObjs));
		return objs;
	}
};