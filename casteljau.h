#include <list>
#include <vector>
#include <algorithm>

#include "point.h"


point get(std::list<point> points, int i)
{
	std::list<point>::iterator it = points.begin();
	for (int x = 0; x < i; x++)
	{
		++it;
	}
	return *it;
}

point evaluate(float t, std::list<point> P)
{
	std::list<point> Q = P;
	while (Q.size() >  1)
	{
		std::list<point> R;
		std::list<point>::iterator p1 = Q.begin();
		std::list<point>::iterator p2 = Q.begin();
		p2++;
		while(p2 != Q.end())
		{
			point p = ((1 - t) * (*p1)) + (t * (*p2));
			R.push_back(p);
			p1++;
			p2++;
			//std::cout << "Pushing Back X Y Z " << p.x << " " << p.y << " " << p.z << std::endl;
		}
		Q = R;
	}

	return *Q.begin();
}

std::vector<point> EvaluateBezierCurve(std::vector<point>ctrl_points, int num_evaluations)
{
	std::list<point> ps(ctrl_points.begin(), ctrl_points.end());
	std::vector<point> curve;

	float offset = 1.0f / num_evaluations;
	curve.push_back(ctrl_points[0]);

	for (int e = 0; e < num_evaluations; e++)
	{
		point p = evaluate(offset * (e + 1), ps);
		curve.push_back(p);
	}

	return curve;
}

float* MakeFloatsFromVector(std::vector<point> curve, int& num_verts, int& num_floats, float r, float g, float b)
{
	num_verts = curve.size();
	if (num_verts == 0)
	{
		return NULL;
	}
	num_floats = num_verts * 6;
	float* floats = (float*)malloc(sizeof(float) * num_floats);
	for (int i = 0; i < curve.size(); i++)
	{
		point p = curve[i];
		floats[i * 6] = p.x;
		floats[i * 6 + 1] = p.y;
		floats[i * 6 + 2] = p.z;
		floats[i * 6 + 3] = r;
		floats[i * 6 + 4] = g;
		floats[i * 6 + 5] = b;
	}
	return floats;
}
