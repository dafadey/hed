#include<iostream>
#define INFO(s) std::cout << #s << std::endl;
#define TAB(l) for(int i=0;i!=l;i++) std::cout << '\t';
struct point
{
	point() : x(0.0), y(0.0), z(0.0) {}
	point(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
	double x,y,z;
};

struct shape
{
	virtual bool inside(const point& p) = 0;
};
struct box : public shape
{
	box(const point& _pmin, const point& _pmax) : pmin(_pmin), pmax(_pmax) {}
	point pmax, pmin;
  virtual bool inside(const point& p)
	{
		if ((p.x-pmax.x)*(p.x-pmin.x) < 0.0 && (p.y-pmax.y)*(p.y-pmin.y) < 0.0 && (p.z-pmax.z)*(p.z-pmin.z) < 0.0)
			return true;
	  return false;
	}
};

struct sphere : public shape
{
	sphere(const point& p, double R) : center(p), r(R) {}
	double r;
	point center;
	virtual bool inside(const point& p)
	{
		double dx=p.x-center.x;
		double dy=p.y-center.y;
		double dz=p.z-center.z;
		if(dx*dx+dy*dy+dz*dz<r*r)
			return true;
		return false;
	}
};

struct query
{
	enum Op {NONE,AND,OR,NOT};
	query* q1;
	query* q2;
	shape* s;
	Op op;
	bool do_query(const point& p)
	{
		//static int level(0);
		//level++;
		//TAB(level)
		//std::cout << "doing query with op=" << op << ", at p(" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
		bool res=false;
		if(op==AND)
			res = q1->do_query(p) & q2->do_query(p);
		if(op==OR)
			res = q1->do_query(p) | q2->do_query(p);
		if(op==NOT)
			res = !(q1->do_query(p));
		if(op==NONE)
			res = s->inside(p);
		//level--;
		//TAB(level)
		//std::cout << "res is " << (res ? "true" : "false") << std::endl;
		return res;
	}
	query() : q2(nullptr), s(nullptr), op(NONE) {}
	query(const shape& _s)
	{
		s = (shape*)&_s;
		q1 = q2 = nullptr;
		op = NONE;
	}
  query& operator&(const query& _q)
	{
		query* q = new query;
		q->q1 = (query*)this;
		q->q2 = (query*)&_q;
		q->op = AND;
		return *q;
	}
	query& operator|(const query& _q)
	{
		query* q = new query;
		q->q1 = (query*)this;
		q->q2 = (query*)&_q;
		q->op = OR;
		return *q;
	}	
 	query& operator!()
	{
		query* q = new query;
		q->q1 = (query*) this;
		q->q2 = nullptr;
		q->op = NOT;
		return *q;
	}
};
