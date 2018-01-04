//all in mm

struct ray
{
	pair pos;
	pair dir;

	void operator init(pair _pos, pair _dir)
	{
		this.pos = _pos;
		this.dir = _dir;
	}

	void unsafe_propagate_to(real new_x, pen p = currentpen)
	{
		if((new_x-pos.x)*dir.x < 0)
		{
			write("unable to propagate ray");
			return;
		}
		pair new_pos = pos + dir/abs(dir.x) * abs(new_x - pos.x);
		draw(pos--new_pos, p);
		pos = new_pos;
	}
	
	void refract_by_angle(real a) // a>0 refract always toward optical axis, a<0 refract outwards optical axis
	{
		dir = (dir.x * cos(a) + (pos.y > 0 ? 1.0 : -1.0) * (dir.y * dir.x > 0 ? 1.0 : -1.0) * dir.y * sin(a), dir.y * cos(a) - (pos.y > 0 ? 1.0 : -1.0) * abs(dir.x) * sin(a));
	}

};

struct lens
{
	real f; //f>0 convex, f<0 concave
	real x;
	real a; // aperture
	
	void operator init(real _f, real _x, real _a)
	{
		this.f = _f;
		this.x = _x;
		this.a = _a;
	}
	
	real delta(real y)
	{
		if(abs(y)<a)
		{
			return atan(abs(y)/f);
		}
		else
			return 0.0;
	}
	
	void unsafe_refract(ray r)
	{
		if(r.pos.x != x)
			r.unsafe_propagate_to(x);
		r.refract_by_angle(delta(r.pos.y));
	}
};


struct setup
{
	lens[] lenses;
	
	real xmin=10e7;
	real xmax=-xmin;
	real ymin=10e7;
	real ymax=-ymin;
	
	real lens_dist(lens l, ray in)
	{
		return (l.x - in.pos.x) * (in.dir.x > 0 ? 1.0 : -1.0);
	}
	int next_lens(ray in)
	{
		int target = -1;
		real ldmin = 10e7;
		for(int i = 0; i != lenses.length; ++i)
		{
			real ld = lens_dist(lenses[i], in);
			write("ld=",ld);
			if(ld <= 0.0)
				continue;
			if(ld < ldmin)
			{
				target = i;
				ldmin = ld;
			}
		}
		write("target=",target);
		return target;
	}
	
	void expandbb(pair p)
	{
		xmin = min(p.x, xmin);
		xmax = max(p.x, xmax);
		ymin = min(p.y, ymin);
		ymax = max(p.y, ymax);
	}
	
	void propagate(ray in, real to, pen p = currentpen)
	{
		int nli = 0;
		while(nli >= 0 && in.dir.x * (in.pos.x - to) <= 0.0)
		{
			expandbb(in.pos);
			write("in.pos0=",in.pos);
			nli = next_lens(in);
			write("nli=",nli);
			if(nli == -1)
				continue;
			in.unsafe_propagate_to(lenses[nli].x, p);
			lenses[nli].unsafe_refract(in);
			write("in.pos`=",in.pos);
		}
		expandbb(in.pos);
		if(in.dir.x * (in.pos.x - to) > 0.0)
			return;
		in.unsafe_propagate_to(to, p);
		expandbb(in.pos);
		return;
	}
	
	void addgrid(real gridx=1.0, real gridy=1.0, real op=0.3)
	{
		if(gridx != 0)
		{
			real x = floor(xmin / gridx) * gridx;
			while(x<xmax)
			{
				draw((x,ymin)--(x,ymax), opacity(op));
				x+=gridx;
			}
		}
		if(gridy != 0)
		{
			real y = floor(ymin / gridy) * gridy;
			while(y<ymax)
			{
				draw((xmin,y)--(xmax,y), opacity(op));
				y+=gridy;
			}
		}
	}
	
	void addopticalaxis(real op=1.0)
	{
		draw((xmin,0)--(xmax,0), opacity(op));
	}
	
	void drawlenses()
	{
		for(int i = 0; i != lenses.length; ++i)
		{
			expandbb((lenses[i].x,lenses[i].a));
			expandbb((lenses[i].x,-lenses[i].a));
			draw((lenses[i].x,lenses[i].a)--(lenses[i].x,-lenses[i].a),Arrows);
		}
	}
	
};

unitsize(1mm);
defaultpen(linewidth(0.1));
setup s;
s.lenses.push(lens(35,0,11.5));
s.lenses.push(lens(50,8,11.5));
for(int i=-3;i!=4;++i)
{
	ray r=ray((-27,0.25*i/3),(43,i));
	s.propagate(r,100,unitrand()*red+unitrand()*green+unitrand()*blue+opacity(0.7));
}

{
	ray r=ray((10,7),(-1,0));
	s.propagate(r,-50);
}

{
	ray r=ray((-5,-7),(1,0));
	s.propagate(r,50);
}

s.drawlenses();
s.addopticalaxis(0.7);
s.addgrid(1.0,1.0,0.1);
s.addgrid(10.0,10.0,0.3);


/*
for(int i=0;i!=128;++i)
{
	ray r=ray((-50+unitrand()*13,unitrand()*13-6.5),(1,0));
	s.propagate(r,100,opacity(0.3));
}
*/
