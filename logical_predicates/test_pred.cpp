#include<iostream>
#include"pred.h"
#include<vector>
#include<cstdlib>
#include<iostream>

int main(int argc, char* argv[])
{
	std::vector<point> pts;
	int N=100000;
	if(argc==2)
		N=atoi(argv[1]);
	for(int i = 0; i != N; i++)
	{
		point p;
		p.x = 4.0 * (double) rand( ) / (double) RAND_MAX - 0.2;
		p.y = 3.0 * (double) rand( ) / (double) RAND_MAX - 0.2;
		p.z = 0.5;//0.0 * 2.0 * (double) rand( ) / (double) RAND_MAX;
		pts.push_back(p);
	}
 	//std::cout << "generated points" << std::endl;
	box b(point(0,0,0),point(3.0,1.5,1.0));
	box b2(point(0.5,0.5,0.1),point(2.8,1.4,0.9));
	sphere s(point(3.0,1.5,0.5),0.5);
	sphere s2(point(3.0,1.6,0.5),0.3);
	sphere s3(point(0.7,0.6,0.5),0.25);
	query q1(b);
	query q2(s);
	query q3(b2);
	query q4(s2);
	query q5(s3);
	query q = q5 | ((!q4) & ((!q3) & (q1 | q2)));
	//query q = (q1 | q2);
	std::vector<point> res;
	for(auto p : pts)
	{
		//std::cout << "doing query" << std::endl;
		if(q.do_query(p))
			res.push_back(p);
	}
	std::cout << "unitsize(10cm);" << std::endl;
	for(auto p : res)
		std::cout << "dot((" << p.x << ", " << p.y << "));" << std::endl;
	return 0;
}
