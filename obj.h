#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <vector>

using namespace std;
#pragma _CRT_SECURE_NO_WARNINGS

class vec3
{
public:
	float x, y, z;

	vec3() {}
	vec3(float l, float m, float r)
	{
		x = l;
		y = m;
		z = r;
	}
	~vec3() {}
};

class vec2
{
public:
	float x, y;

	vec2() {}
	vec2(float l, float r)
	{
		x = l;
		y = r;
	}
	~vec2() {}
};

struct vertex
{
public:
	vec3 vc;
	vec2 tc;
	vec3 nc;

	vertex() {}
	vertex(vec3 vc_in, vec2 tc_in, vec3 nc_in)
	{
		vc = vc_in;
		tc = tc_in;
		nc = nc_in;
	}
	~vertex() {}
};
struct triangle
{
public:
	vertex verts[3];

	triangle() {}
	triangle(vertex v0, vertex v1, vertex v2)
	{
		verts[0] = v0;
		verts[1] = v1;
		verts[2] = v2;
	}
	~triangle() {}
};

class Material
{
public:
	char mtl_name[256];
	char fil_name[256];

	Material() {}
	Material(char* n, char* f)
	{
		strcpy(mtl_name, n);
		strcpy(fil_name, f);
	}
	~Material()
	{
	}
};

class Object
{
public:
	unsigned int VAO;
	unsigned int VBO;
	vector<triangle> tris;
	Material mtl;
	GLuint texture;


	Object() {}
	Object(Material m)
	{
		strcpy(mtl.fil_name, m.fil_name);
		strcpy(mtl.mtl_name, m.mtl_name);
	}
	~Object()
	{
	}
};

int mtl_parse(char* filename, vector<Material>* mtls)
{
	std::cout << "Creating Material" << std::endl;
	// Open up the mtl file
	FILE* mtlFile = fopen(filename, "r");
	bool endOfFileReached = false;
	// if the file doesnt exist then return an error
	if (mtlFile == NULL)
	{
		std::cout << "Cannot Open Associated MTL File >" << filename << "<" << std::endl;
		return -1;
	}
	// While there is data to read from the file
	while (!endOfFileReached)
	{
		// string to file contents in
		char command[512];
		// Get the first word of the line
		int returnValue = fscanf(mtlFile, "%s", command);
		// If end of file reached then stop the loopinmg
		if (returnValue == EOF)
		{
			endOfFileReached = true;
		}
		// if newmtl is spotted
		else if (strcmp(command, "newmtl") == 0)
		{
			// Create new Material
			Material newMat;
			char matName[512];
			// Scan the name of this new material and copy it into the Material object
			fscanf(mtlFile, "%s\n", matName);
			strcpy(newMat.mtl_name, matName);
			mtls->push_back(newMat);
			std::cout << "Material with name " << matName << " Created" << std::endl;
		}
		// if map_Kd is spotted
		else if (strcmp(command, "map_Kd") == 0)
		{
			// Create a string for its file path and copy the cooresponding word from the mtl file
			char mtlFilePath[512];
			fscanf(mtlFile, "%s\n", mtlFilePath);
			// convert the char array string into an std::string
			std::string filePath(filename);
			// Get everything before the last / within the file path
			filePath = filePath.substr(0, filePath.find_last_of("\\/"));
			// Append a / to it to make it a valid path again
			filePath.append("/");
			// Create a new std::string using the copied file path
			std::string mtlName(mtlFilePath);
			// append the name to the file math
			filePath.append(mtlName);
			// finally we convert it back to a char array string and copy the data into a char array before pushing it into the material object
			char finalPath[512];
			strcpy(finalPath, filePath.c_str());
			strcpy(mtls->at(mtls->size() - 1).fil_name, finalPath);
			std::cout << "Adding file path " << mtlFilePath << " To material " << mtls->at(mtls->size() - 1).mtl_name << std::endl;
		}
	}
	//success
	return 1;
}


int obj_parse(const char* filename, vector<Object>* objs)
{
	vector<Material> materials;
	vector<vec3> vecs;
	vector<vec2> uvs;
	vector<vec3> normals;
	// Open the file
	FILE* objFile = fopen(filename, "r");
	bool endOfFileReached = false;
	// Check if file can be opened
	if (objFile == NULL)
	{
		std::cout << "Cannot Open File Provided" << std::endl;
		return -1;
	}
	// Check every line
	while (!endOfFileReached)
	{
		// Char array to store the first word
		char command[512] = { 0 };
		// Scan the first word of the line
		int returnValue = fscanf(objFile, "%s", command);
		// Check the end of the file hasnt been reached
		if (returnValue == EOF)
		{
			endOfFileReached = true;
		}
		else
		{
			// Check if pointing to an mtl file
			if (strcmp(command, "mtllib") == 0)
			{
				char mtlFileName[512] = { 0 };
				// Parse MTL file
				// First we convert the name into a full file path to find the file
				fscanf(objFile, "%s\n", mtlFileName);
				std::string filePath(filename);
				filePath = filePath.substr(0, filePath.find_last_of("\\/"));
				filePath.append("/");
				std::string mtlName(mtlFileName);
				filePath.append(mtlName);
				char finalPath[512];
				strcpy(finalPath, filePath.c_str());
				// Then we use that file path to parse the mtl file
				int mtlReturnValue = mtl_parse(finalPath, &materials);
			}
			// Check if verticies are specified 
			if (strcmp(command, "v") == 0)
			{
				float x, y, z;
				// Scan for 3 floats
				fscanf(objFile, "%f %f %f\n", &x, &y, &z);
				// Store them in a new vec3
				vec3 newVertex = vec3(x, y, z);
				vecs.push_back(newVertex);
			}
			if (strcmp(command, "vn") == 0)
			{
				float x, y, z;
				// Scan for 3 floats
				fscanf(objFile, "%f %f %f\n", &x, &y, &z);
				// Store them in a new vec3
				vec3 newNormal = vec3(x, y, z);
				normals.push_back(newNormal);
			}
			// Check if uv coordinates are specified
			if (strcmp(command, "vt") == 0)
			{
				float u, v;
				// Scan 2 floats
				fscanf(objFile, "%f %f\n", &u, &v);
				// Store them in a vec2
				vec2 newUv = vec2(u, v * -1);
				uvs.push_back(newUv);
			}
			if (strcmp(command, "usemtl") == 0)
			{
				// Create a new object
				Object newObject;
				char materialName[512];
				// Get the name of the material
				fscanf(objFile, "%s\n", materialName);
				bool materialFound = false;
				// Find the material
				for (int i = 0; i < materials.size(); i++)
				{
					if (strcmp(materials[i].mtl_name, materialName) == 0)
					{
						// If its found copy it into the object
						newObject.mtl = materials[i];
						materialFound = true;
						objs->push_back(newObject);
					}
				}
				// If its not found give an error warning and return an error
				if (!materialFound)
				{
					std::cout << "Material Missing when 'usemtl' called" << std::endl;
					return -1;
				}

			}
			// If a new triangle/face is specified
			if (strcmp(command, "f") == 0)
			{
				int vertexs[3], uv[3], normal[3];
				// Scan all the values and store them in 3 different arrays
				fscanf(objFile, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexs[0], &uv[0], &normal[0], &vertexs[1], &uv[1], &normal[1], &vertexs[2], &uv[2], &normal[2]);
				vertex first(vecs.at(vertexs[0] - 1), uvs.at(uv[0] - 1), normals.at(normal[0] - 1));
				vertex second(vecs.at(vertexs[1] - 1), uvs.at(uv[1] - 1), normals.at(normal[1] - 1));
				vertex third(vecs.at(vertexs[2] - 1), uvs.at(uv[2] - 1), normals.at(normal[2] - 1));

				// Store all the information in a new triangle object
				triangle tri(first, second, third);
				// Store the new triangle in the list
				objs->back().tris.push_back(tri);
			}
		}
	}

	//success
	return 1;
}

