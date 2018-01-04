//glfw version
#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include <pthread.h>
#if defined(__APPLE__) || defined(MACOSX)
#include <GL/glfw.h>
#else
#include <GL/glfw.h>
#endif
//#include <rendercheck_gl.h>

//using namespace std;
static char* title;

//2d colormaps:
static GLuint* texture;
static int*    texUPD;
static int*    texSZ;
static float*    texMAG;
static float*    texPOS;
static float** texturesDataArrs;
static float** DataArrs;

//particles and 1d plots structures:
typedef float color[3]; 
typedef float bounds[4]; //x0,y0,x1,y1

static int*    particleCloudNum; // number of particle clouds for all tiles
static bounds*    particleBounds; // bounds for all clouds for each tile
static int** particlesSZ; // number of particles for each cloud
static float*** particles; // particles pointers
static color** particlesColor; // particles color

//1d plots:
static int*    plotNum; // number of plots clouds for all tiles
static bounds*    plotBounds; // bounds for all plots for each tile
static int**    plotsSZ; // size of plots
static float***    plots; // plots data pointers
static color** plotsColor; // plot color

static int* close_win;
static pthread_mutex_t				close_win_mutex;
static pthread_mutex_t				data_update_mutex;

static int XSize = 512;
static int YSize = 512;
static int XnewSize = XSize;
static int YnewSize = YSize;
static int Nx;
static int Ny;
static int count;
static int side_max;
static int xmouse, ymouse;
static float pdx_drag, pdy_drag;
static volatile int initedfirst=0;
static	int *inited;
static int *tileUsage; // array of tile size with usage flag: -1 - not used, 0 - 2d data 1 - points in X-Y, 2 - 1d plots

void	showMessage(GLfloat, GLfloat, char*);

static float red(float val, float max)
{
	if(val>0)	return sqrt(val/max);
	else return (val/max)*(val/max)*0.35;
}
static float green(float val, float max)
{
	if(val<0) return (val/max)*(val/max)*0.75+sqrt(-val/max)*0.25;
	else	return (val/max)*(val/max);
}
static float blue(float val, float max)
{
	if(val<0)	return sqrt(-val/max);
	else return 0.0;
}

static bool draG=false;
static int wheel_state=0;
static int xi=0;
static int yi=0;
static int sq=0;
static float xxx=0;
static float yyy=0;
static	int vh=0;

static void mouse_wheel(int new_wheel_pos)
{
		int wheel=new_wheel_pos-wheel_state;
		int x,y;
		glfwGetMousePos(&x, &y);
		float xa=float(x)/float(XnewSize);
		float ya=float(y)/float(YnewSize);
		int ii=floor(xa*float(side_max));
		int jj=floor(ya*float(side_max));
		int number=ii+jj*side_max;
		float pmag=texMAG[number];
		texMAG[number]+=float(wheel)/13.0;
		float lxa=xa*float(side_max)-float(ii);
		float lya=ya*float(side_max)-float(jj);
		texPOS[number*2]+=lxa*(pmag-texMAG[number]);
		texPOS[number*2+1]+=lya*(pmag-texMAG[number]);
		wheel_state=new_wheel_pos;
		//printf("texposx=%f,\ttexposy=%f,\tmag=%f\n",texPOS[number*2],texPOS[number*2+1],texMAG[number]);
}

static void mouse_drag(int button, int PressRelease)
{
	if(button==GLFW_MOUSE_BUTTON_LEFT)
	{
		glfwGetMousePos(&xmouse, &ymouse);
		pdx_drag=0;
		pdy_drag=0;
		if (PressRelease==GLFW_PRESS) draG=true;
		else if (PressRelease==GLFW_RELEASE) draG=false;
	}
	if((button==GLFW_MOUSE_BUTTON_RIGHT)&(PressRelease==GLFW_PRESS)) vh=(vh<1)?vh+1:-1;
}

static void mouse_move(int x, int y)
{
	if(draG==true)
	{
		float xa=float(xmouse)/float(XnewSize);
		float ya=float(ymouse)/float(YnewSize);
		int ii=floor(xa*float(side_max));
		int jj=floor(ya*float(side_max));
		int number=ii+jj*side_max;

		int dx=x-xmouse;
		int dy=y-ymouse;
		//printf("GL: drag x=%d y=%d\n",dx,dy);
		float dxa=float(dx)/float(XnewSize)*float(side_max)*texMAG[number];
		float dya=float(dy)/float(YnewSize)*float(side_max)*texMAG[number];
		texPOS[number*2]-=dxa-pdx_drag;
		texPOS[number*2+1]-=dya-pdy_drag;	
		pdx_drag=dxa;
		pdy_drag=dya;
		//printf("texposx=%f,\ttexposy=%f\n",texPOS[number*2],texPOS[number*2+1]);
	}
//	else
	{
		float xa=float(x)/float(XnewSize);
		float ya=float(y)/float(YnewSize);
		int ii=floor(xa*float(side_max));
		int jj=floor(ya*float(side_max));
		sq=ii+jj*side_max;

		xxx=xa*float(side_max)-floor(xa*float(side_max));
		yyy=ya*float(side_max)-floor(ya*float(side_max));
		xi=floor((xxx*texMAG[sq]+texPOS[sq*2]-floor(xxx*texMAG[sq]+texPOS[sq*2]))*texSZ[sq*2]);
		yi=floor((yyy*texMAG[sq]+texPOS[sq*2+1]-floor(yyy*texMAG[sq]+texPOS[sq*2+1]))*texSZ[sq*2+1]);

	}

}

static void win_sz(int x, int y)
{
		glViewport(0,0,x,y);
		XnewSize=x;
		YnewSize=y;
//		printf("GL (Resize): XSize=%d\n",XSize);
}

static int 
onwinclose(void)
{
	inited[0]=0;
	return GL_FALSE;
}


bool ifInBounds(float x, float y, bounds bb)
{
	if((x>=bb[0])&(x<=bb[2])&(y>=bb[1])&(y<=bb[3]))
		return true;
	else
		return false;
}

void transformTex(float* x, float* y, float texposx, float texposy, float texmag, float dx, float dy)
{
	*x+=-texposx*dx;	
	*y+=-texposy*dy;
	*x/=texmag;
	*y/=texmag;
}

bool crossLineWithBB(float* x0, float* y0, float* x1, float* y1, bounds bb)
{
	int i=0;
	float* p[4];
	float po[4];
	p[0]=x0;
	p[1]=y0;
	p[2]=x1;
	p[3]=y1;
	//find good point
	if(ifInBounds(*x0,*y0,bb))
	{
		po[i*2]=*x0;
		po[i*2+1]=*y0;
		i++;
	}
	if(ifInBounds(*x1,*y1,bb))
	{
		po[i*2]=*x1;
		po[i*2+1]=*y1;
		i++;
	}
	int j=0;
	while((i<2)&(j<4))
	{
		//j=0,2 : /(x0-x1) j=0 xmin=bb[0], j=2 xmax=bb[2]
		//j=1,3 : /(y0-y1) j=0 ymin=bb[1], j=3 ymax=bb[3]
		float c=((bb[j]-*p[2+j%2])**p[1-j%2]+*p[2+1-j%2]*(*p[j%2]-bb[j]))/(*p[j%2]-*p[2+j%2]);
		//printf("j=%d cross is %f line(%g,%g)--(%g,%g) crossing %g\n",j,c,*p[j%2],*p[1-j%2],*p[2+j%2],*p[2+1-j%2],bb[j]);
		if(((c-bb[2+1-j%2])*(c-bb[1-j%2])<=0)&((c-*p[1-j%2])*(c-*p[2+1-j%2])<=0))
		{
			po[i*2+j%2]=bb[j];
			po[i*2+1-j%2]=c;
			//printf("real cross found\n");
			i++;
		}
		j++;
	}
	if(i==1);
		//printf("simpledraw2D: error occured during crossing line (%g,%g)--(%g,%g) with bb (%g,%g)-(%g,%g) found only one cross\n",*x0,*y0,*x1,*y1,bb[0],bb[1],bb[2],bb[3]);
	else if(i==2)
	{
		*x0=po[0];
		*y0=po[1];
		*x1=po[2];
		*y1=po[3];
		return true;
	}
	return false;
		
}

static void 
display(void)
{
	glfwSwapBuffers();
  glMatrixMode (GL_MODELVIEW);
  glDisable(GL_DEPTH_TEST);
  glClearColor(.3, .3, .3, 0);
  glClear(GL_COLOR_BUFFER_BIT);

	int ii,jj;
	int x=0;
	int y=0;
	int dx=floor(float(XSize)/float(side_max));
	int dy=floor(float(YSize)/float(side_max));

	bounds tbb; // setting tile bounds
		tbb[0]=0;
		tbb[1]=0;
		tbb[2]=dx;
		tbb[3]=dy;

	for(jj=0;jj<side_max;jj++)
	{
		x=0;
		for(ii=0;ii<side_max;ii++)
		{
			int number=ii+jj*side_max;
			if(number<count)
			{
				float xc=x+xxx*dx;
				float yc=y+yyy*dy;
				if(tileUsage[number]==0)
				{
					if((vh!=0)&(number==sq)) glColor3f(0.5,0.5,0.5);
					else glColor3f(1,1,1);
					float mag=texMAG[number];
					float xtex=texPOS[number*2];
					float ytex=texPOS[number*2+1];
					glBindTexture(GL_TEXTURE_2D, texture[number]);
					glBegin(GL_QUADS);
					glTexCoord2f(xtex, ytex);
					glVertex2f(x+1, y+1);
					glTexCoord2f(xtex+1.0*mag, ytex);
					glVertex2f(x+dx-1, y+1);
					glTexCoord2f(xtex+1.0*mag, ytex+1.0*mag);
					glVertex2f(x+dx-1, y+dy-1);
					glTexCoord2f(xtex, ytex+1.0*mag);
					glVertex2f(x+1, y+dy-1);
					glEnd();

					glBindTexture(GL_TEXTURE_2D, 0);
					if(number==sq)
					{
						glColor3f(1,0,0);
						glBegin(GL_QUADS);
						glVertex2f(xc-3+1, yc-3+1);
						glVertex2f(xc+3-1, yc-3+1);
						glVertex2f(xc+3-1, yc+3-1);
						glVertex2f(xc-3+1, yc+3-1);
						glEnd();

						glColor3f(0.5,0.5,0.5);
						if(vh==1)
						{
							glBegin(GL_LINES);
							glVertex2f(1+x, yc+1);
							glVertex2f(1+x+dx, yc+1);
							glEnd();

							int i;
							float max=-1.0e11;
							float min=1.0e11;
							for(i=0;i<texSZ[sq*2];i++)
							{
								int ii=(float(i)/float(texSZ[sq*2])*texMAG[sq]+texPOS[sq*2]-floor(float(i)/float(texSZ[sq*2])*texMAG[sq]+texPOS[sq*2]))*float(texSZ	[sq*2]);
								max=(max>DataArrs[sq][yi*texSZ[sq*2]+ii])?max:DataArrs[sq][yi*texSZ[sq*2]+ii];
								min=(min<DataArrs[sq][yi*texSZ[sq*2]+ii])?min:DataArrs[sq][yi*texSZ[sq*2]+ii];
							}
							glColor3f(0.5,0.5,0.5);
							glBegin(GL_LINES);
							glVertex2f(1+x, y+1+max/(max-min)*dy);
							glVertex2f(1+x+dx, y+1+max/(max-min)*dy);
							glEnd();
							for(i=0;i<11;i++)
							{
		 						sprintf(title,"%.2e\n",min+(max-min)*float(i)/10.0);
								showMessage(1+x,y+dy-3-i*(dy-6)/10,title);
							}
							glBegin(GL_LINES);
							glColor3f(1,1,1);
							for(i=0;i<texSZ[sq*2]-1;i++)
							{
								int ii=(float(i)/float(texSZ[sq*2])*texMAG[sq]+texPOS[sq*2]-floor(float(i)/float(texSZ[sq*2])*texMAG[sq]+texPOS[sq*2]))*float(texSZ	[sq*2]);
								int ii_1=(float(i+1)/float(texSZ[sq*2])*texMAG[sq]+texPOS[sq*2]-floor(float(i+1)/float(texSZ[sq*2])*texMAG[sq]+texPOS[sq*2]))*float(texSZ	[sq*2]);
								if((ii_1>=0)&(ii_1<texSZ[sq*2])&(ii>=0)&(ii<texSZ[sq*2]))
								{
									glVertex2f(1+x+dx*float(i)/float(texSZ[sq*2]), y+1+(max-DataArrs[sq][yi*texSZ[sq*2]+ii])/(max-min)*dy);
									glVertex2f(1+x+dx*float(i+1)/float(texSZ[sq*2]), y+1+(max-DataArrs[sq][yi*texSZ[sq*2]+ii_1])/(max-min)*dy);
								}
							}
							glEnd();

						}

						if(vh==-1)
						{
							glBegin(GL_LINES);
							glVertex2f(1+xc, 1+y);
							glVertex2f(1+xc, dy+y+1);
							glEnd();

							int i;
							float max=-1.0e11;
							float min=1.0e11;
							for(i=0;i<texSZ[sq*2+1];i++)
							{
								int ii=(float(i)/float(texSZ[sq*2+1])*texMAG[sq]+texPOS[sq*2+1]-floor(float(i)/float(texSZ[sq*2+1])*texMAG[sq]+texPOS[sq*2+1]))*float(texSZ[sq*2+1]);
								max=(max>DataArrs[sq][ii*texSZ[sq*2]+xi])?max:DataArrs[sq][ii*texSZ[sq*2]+xi];
								min=(min<DataArrs[sq][ii*texSZ[sq*2]+xi])?min:DataArrs[sq][ii*texSZ[sq*2]+xi];
							}
							glColor3f(0.5,0.5,0.5);
							glBegin(GL_LINES);
							glVertex2f(1+x, y+1+max/(max-min)*dy);
							glVertex2f(1+x+dx, y+1+max/(max-min)*dy);
							glEnd();
							for(i=0;i<11;i++)
							{
		 						sprintf(title,"%.2e\n",min+(max-min)*float(i)/10.0);
								showMessage(1+x,y+dy-3-i*(dy-6)/10,title);
							}
							glBegin(GL_LINES);
							glColor3f(1,1,1);
							for(i=0;i<texSZ[sq*2+1]-1;i++)
							{
								int ii=(float(i)/float(texSZ[sq*2+1])*texMAG[sq]+texPOS[sq*2+1]-floor(float(i)/float(texSZ[sq*2+1])*texMAG[sq]+texPOS[sq*2+1]))*float(texSZ[sq*2+1]);
								int ii_1=(float(i+1)/float(texSZ[sq*2+1])*texMAG[sq]+texPOS[sq*2+1]-floor(float(i+1)/float(texSZ[sq*2+1])*texMAG[sq]+texPOS[sq*2+1]))*float(texSZ[sq*2+1]);
								if((ii_1>=0)&(ii_1<texSZ[sq*2+1])&(ii>=0)&(ii<texSZ[sq*2+1]))
								{
									glVertex2f(1+x+dx*float(i)/float(texSZ[sq*2+1]), y+1+(max-DataArrs[sq][ii*texSZ[sq*2]+xi])/(max-min)*dy);
									glVertex2f(1+x+dx*float(i+1)/float(texSZ[sq*2+1]), y+1+(max-DataArrs[sq][ii_1*texSZ[sq*2]+xi])/(max-min)*dy);
								}
							}
							glEnd();
						}
					}
				}
				// particles section and 1d graphs
				if(tileUsage[number]==1) // particles
				{
					glBegin(GL_POINTS);
					//printf("tile #%d plotting point clouds N=%d\n",number,particleCloudNum[number]);
					for(int i=0;i<particleCloudNum[number];i++) // loop over clouds
					{
						bounds bb;
						for(int j=0;j<4;j++) bb[j]=particleBounds[number][j];
						color* C=&particlesColor[number][i];
						glColor3f((*C)[0],(*C)[1],(*C)[2]);
						//printf("\tcloud[%d] has %d particles with color (%g, %g, %g)\n",i,particlesSZ[number][i],*C[0],*C[1],*C[2]);
						for(int j=0;j<particlesSZ[number][i];j++) // loop over points
						{
							//draw
							float px=particles[number][i][j*2];
							float py=particles[number][i][j*2+1];
							//if(j<13)
							//	printf("x=%g,y=%g\n",particles[number][i][j*2],particles[number][i][j*2+1]);
							//glVertex2f(xc-3+y, yc-3+x);
							float spx=(px-bb[0])/(bb[2]-bb[0]+DBL_MIN)*dx;
							float spy=(py-bb[1])/(bb[3]-bb[1]+DBL_MIN)*dy;
							transformTex(&spx,&spy,texPOS[number*2],texPOS[number*2+1],texMAG[number],dx,dy);
							if(ifInBounds(spx,spy,tbb))
								glVertex2f(x+spx, y+spy);
						}
					}
					glEnd();
				}
				if(tileUsage[number]==2) // 1d plots
				{
					glBegin(GL_LINES);
					//printf("tile #%d plotting point clouds N=%d\n",number,particleCloudNum[number]);
					for(int i=0;i<plotNum[number];i++) // loop over clouds
					{
						bounds bb;
						for(int j=0;j<4;j++) bb[j]=plotBounds[number][j];
						color* C=&plotsColor[number][i];
						glColor3f((*C)[0],(*C)[1],(*C)[2]);
						//printf("\tcloud[%d] has %d particles with color (%g, %g, %g)\n",i,particlesSZ[number][i],*C[0],*C[1],*C[2]);
						for(int j=0;j<plotsSZ[number][i]-1;j++) // loop over points
						{
							//draw
							float px0=plots[number][i][j*2];
							float py0=plots[number][i][j*2+1];
							float px1=plots[number][i][(j+1)*2];
							float py1=plots[number][i][(j+1)*2+1];
							//if(j<13)
							//	printf("x=%g,y=%g\n",particles[number][i][j*2],particles[number][i][j*2+1]);
							//glVertex2f(xc-3+y, yc-3+x);
							float spx0=(px0-bb[0])/(bb[2]-bb[0]+DBL_MIN)*dx;
							float spy0=(py0-bb[1])/(bb[3]-bb[1]+DBL_MIN)*dy;
							transformTex(&spx0,&spy0,texPOS[number*2],texPOS[number*2+1],texMAG[number],dx,dy);
							float spx1=(px1-bb[0])/(bb[2]-bb[0]+DBL_MIN)*dx;
							float spy1=(py1-bb[1])/(bb[3]-bb[1]+DBL_MIN)*dy;
							transformTex(&spx1,&spy1,texPOS[number*2],texPOS[number*2+1],texMAG[number],dx,dy);
							if(ifInBounds(spx0,spy0,tbb)&ifInBounds(spx1,spy1,tbb)) //both points inside
							{
								glVertex2f(x+spx0, y+spy0);
								glVertex2f(x+spx1, y+spy1);
							}
							else// if(ifInBounds(spx0,spy0,tbb)^ifInBounds(spx1,spy1,tbb)) // only one point is inside
							{
								if(crossLineWithBB(&spx0,&spy0,&spx1,&spy1,tbb))
								{
									glVertex2f(x+spx0, y+spy0);
									glVertex2f(x+spx1, y+spy1);
								}
							}
						}
					}
					glEnd();
				}
			}
			x+=dx;
		}
		y+=dy;
	}


		if((xi>=0)&(xi<texSZ[sq*2])&(yi>=0)&(yi<texSZ[sq*2+1])) sprintf(title,"f[%d](%d,%d)=%f",sq,xi,yi,DataArrs[sq][xi+yi*texSZ[sq*2]]);

		glfwSetWindowTitle(title);
  	//glMatrixMode (GL_MODELVIEW);

}

static void idle()
{
 	  int i;
    for(i=0;i<count;i++)
    {
        if(texUPD[i]==0)
        {
		    glBindTexture(GL_TEXTURE_2D, texture[i]);
#ifdef GL_VERSION_1_1
		    glTexImage2D(GL_TEXTURE_2D, 0, 4,texSZ[i*2],texSZ[i*2+1], 0, GL_RGBA, GL_FLOAT, texturesDataArrs[i]);
#else
		    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB,texSZ[i*2],texSZ[i*2+1], 0, GL_RGBA, GL_FLOAT, texturesDataArrs[i]);
#endif
		    texUPD[i]=1;
		}
        
    }
//	glutPostRedisplay();
}

static pthread_t					GLloop_th_id;
void* GLloop(void*)
{

//	printf("GLloop: activated\n");
	int i,j,k;
	texturesDataArrs=new float*[count];
	DataArrs=new float*[count];
	texUPD=new int[count];
	texSZ=new int[count*2];
	texMAG=new float[count];
	texPOS=new float[count*2];
	for(k=0;k<count;k++)
	{
		texSZ[k*2]=Nx;
		texSZ[k*2+1]=Ny;
		texMAG[k]=1.0;
		texPOS[k*2]=0.0;
		texPOS[k*2+1]=0.0;
	}
	for(i=0;i<count;i++) texUPD[i]=0;
	for(i=0;i<count;i++) texturesDataArrs[i]=new float[texSZ[i*2]*texSZ[i*2+1]*4];
	for(i=0;i<count;i++) DataArrs[i]=texturesDataArrs[i];
	texture=new GLuint[count];

	for(k=0;k<count;k++)
	{
		for(j=0;j<texSZ[k*2+1];j++)
		{
		  for(i=0;i<texSZ[k*2];i++)
		  {
			 	float x=float(i-Nx/2)/float(Nx);
				float y=float(j-Ny/2)/float(Ny);
				texturesDataArrs[k][(j*texSZ[k*2]+i)*4]=exp(-(x*x+y*y)*13.0*(1+k));
				texturesDataArrs[k][(j*texSZ[k*2]+i)*4+1]=0.0;
				texturesDataArrs[k][(j*texSZ[k*2]+i)*4+2]=0.0;
				texturesDataArrs[k][(j*texSZ[k*2]+i)*4+3]=0.0;
		  }
		}
		//texturesDataArrs[k][((64+25)*128+64+33)*4+1]=1.0;
		//texturesDataArrs[k][((64-33)*128+64+47)*4+2]=1.0;
	}
	glfwInit();
	//glfwSwapInterval(10);
	glfwOpenWindow( XSize, YSize, 8, 8, 8, 8, 8, 8, GLFW_WINDOW);
	glfwSetWindowTitle("fadey draw");
	glfwSetWindowPos( 13, 13);
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho (0, XSize, YSize, 0, -1, 1);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glfwSetWindowSizeCallback(win_sz);//	glutReshapeFunc(win_sz);
	glDisable(GL_LIGHTING);
	glfwSetMouseButtonCallback(mouse_drag);
	glfwSetMousePosCallback(mouse_move);
	glfwSetMouseWheelCallback(mouse_wheel);
	glfwSetWindowCloseCallback(onwinclose);

	int ii, jj;
	side_max=-floor(-sqrt(float(count)));
//  glActiveTexture(GL_TEXTURE0);
  glGenTextures(count, texture);
	for(jj=0;jj<side_max;jj++)
	{
		for(ii=0;ii<side_max;ii++)
		{
			int number=ii+jj*side_max;
			if(number<count)
			{
				//printf("making texture number %d\n",number);
				glBindTexture(GL_TEXTURE_2D, texture[number]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#ifdef GL_VERSION_1_1
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, texSZ[number*2], texSZ[number*2+1], 0, GL_RGBA, GL_FLOAT, texturesDataArrs[number]);
#else
				glTexImage2D(GL_TEXTURE_2D, 0, 4, texSZ[number*2], texSZ[number*2+1], 0, GL_RGBA, GL_FLOAT, texturesDataArrs[number]);
#endif
				texUPD[number]=1;				
			}
		}
	}
	//printf("side_max=%d\n",side_max);
	pthread_mutex_unlock(&close_win_mutex);

	while(inited[0]==1)
	{
		idle();
		display();
//		glfwSleep(0.05);
	}
	printf("fadey_draw: quitting... preparing to destroy data\n");
	pthread_mutex_lock(&data_update_mutex);
	printf("fadey_draw: quitting... destroying window\n");
	glfwCloseWindow();
	printf("fadey_draw: quitting... destroying data\n");
	delete[] texture;
	delete[] texUPD;
	delete[] texSZ;
	delete[] texMAG;
	delete[] texPOS;
	for(i=0;i<count;i++) delete[] texturesDataArrs[i];
	delete[] texturesDataArrs;
	delete[] close_win;
	delete[] title;
	pthread_mutex_unlock(&data_update_mutex);
	printf("fadey_draw: clean exit\n");
	pthread_mutex_unlock(&close_win_mutex);
	return 0;
}

void fadey_init(int Nx_, int Ny_, int count_)
{
	if(initedfirst==0)
	{
		inited=new int[1];
		inited[0]=0;
		initedfirst=1;
	}
	if(inited[0]==0)
	{
		tileUsage=new int[count_];
		for(int i=0;i<count_;i++) tileUsage[i]=-1; // do not use tiles on init.
		title=new char[25];
		inited[0]=1;
		Nx=Nx_;
		Ny=Ny_;
		count=count_;
		pthread_create(&GLloop_th_id, NULL, &GLloop, NULL);
		close_win=new int[1];
		close_win[0]=0;

		particleCloudNum=new int[count_];
		particleBounds=new bounds[count_];
		particles=new float**[count_];
		particlesColor=new color*[count_];
		particlesSZ=new int*[count_];
		for(int i=0;i<count_;i++)
		{
			particleCloudNum[i]=0;
			for(int j=0;j<4;j++)
				particleBounds[count_][j]=(j<2)?DBL_MAX:-DBL_MAX;
		}

		plotNum=new int[count_];
		plotBounds=new bounds[count_];
		plots=new float**[count_];
		plotsColor=new color*[count_];
		plotsSZ=new int*[count_];
		for(int i=0;i<count_;i++)
		{
			plotNum[i]=0;
			for(int j=0;j<4;j++)
				plotBounds[count_][j]=(j<2)?DBL_MAX:-DBL_MAX;
		}

		pthread_mutex_init(&close_win_mutex,NULL);
		pthread_mutex_init(&data_update_mutex,NULL);
		pthread_mutex_lock(&close_win_mutex);
		pthread_mutex_lock(&close_win_mutex);
		printf("init: bye\n");
	}
}


void fadey_draw(float* DataArr, int Nx_, int Ny_, int count_)
{
	pthread_mutex_lock(&data_update_mutex);
	if(inited[0]==1)
	{  
		int i,j;
		int k=count_;
		tileUsage[k]=0;
		DataArrs[k]=DataArr;
		if ((Nx_!=texSZ[k*2])||(Ny_!=texSZ[k*2+1]))
		{
			delete[] texturesDataArrs[k];
			texSZ[k*2]=Nx_;
			texSZ[k*2+1]=Ny_;
			texturesDataArrs[k]=new float[texSZ[k*2]*texSZ[k*2+1]*4];
//			texturesDataArrs[k][(j*texSZ[k*2]+i)*4]=DataArr[j*texSZ[k*2]+i];
	//		printf("texture %d new size is (%d x %d)\n",k,texSZ[k*2],texSZ[k*2+1]);
		}
		float max=0.0;
		float min=0.0;
		for(j=0;j<texSZ[k*2+1];j++)
		{
			for(i=0;i<texSZ[k*2];i++)
			{
				if(DataArr[j*texSZ[k*2]+i]>max) max=DataArr[j*texSZ[k*2]+i];
				if(DataArr[j*texSZ[k*2]+i]<max) min=DataArr[j*texSZ[k*2]+i];
			}
		}
		if(-min>max) max=-min;
		for(j=0;j<texSZ[k*2+1];j++)
		{
			for(i=0;i<texSZ[k*2];i++)
			{
				texturesDataArrs[k][(j*texSZ[k*2]+i)*4]=red(DataArr[j*texSZ[k*2]+i],max);
				texturesDataArrs[k][(j*texSZ[k*2]+i)*4+1]=green(DataArr[j*texSZ[k*2]+i],max);
				texturesDataArrs[k][(j*texSZ[k*2]+i)*4+2]=blue(DataArr[j*texSZ[k*2]+i],max);
				texturesDataArrs[k][(j*texSZ[k*2]+i)*4+3]=0.0;
			}
		}
		texUPD[k]=0;
  }
	pthread_mutex_unlock(&data_update_mutex);
}

void fadey_draw_particles_reset_bounds(int tile)
{
	for(int j=0;j<4;j++)
		particleBounds[tile][j]=(j<2)?DBL_MAX:-DBL_MAX;
}


void fetch_data(int* plotNum, bounds* bb, int** SZ, float*** localdata, color** Color, int index, float* userData, int N, int tile, double r=-1,  double g=-1, double b=-1)
{
	if(index>=plotNum[tile])
	{
		//make new cloud: create SZ, localdata and particleColors then copy previous data there
		int* pSZ=new int[index+1];
		color *pC=new color[index+1];
		float** pp=new float*[index+1];
		printf("preparing structures for new particle cloud at index %d\n",index);
		for(int i=0;i<plotNum[tile];i++)
		{
			pSZ[i]=SZ[tile][i];
			for(int j=0;j<3;j++)
				pC[i][j]=Color[tile][i][j];
			pp[i]=localdata[tile][i];
		}
		printf("\told data stored\n");
		for(int i=plotNum[tile];i<index+1;i++)
		{
			pSZ[i]=-1;
			for(int j=0;j<3;j++)
				pC[i][j]=1.0; // add default white color for newly created pointCloud
		}
		printf("\tnew data added\n");
		if(plotNum[tile]!=0)
		{
			delete[] SZ[tile]; 
			delete[] Color[tile]; 
			delete[] localdata[tile];
		}
		else
			printf("\tdoing this first time thus do not releasing memory\n");
		SZ[tile]=pSZ;
		Color[tile]=pC;
		localdata[tile]=pp;
		plotNum[tile]=index+1;
	}
	if(N!=SZ[tile][index])
	{
		//printf("allocating space for new particle cloud coordinates of size %d\n", N);
		//if N > localdataNum create new localdata[tile][index] no copy needed color is preserved
		if(SZ[tile][index]!=-1)
			delete[] localdata[tile][index];
		else
			printf("allocating space for new particle cloud first time thus do not releasing memory\n");
		localdata[tile][index]=new float[N*2];
		SZ[tile][index]=N; // update number
	}
	// now we are ready to fill data (we are sure that we reserved memory for all data fields):
	for(int i=0;i<N;i++)
	{
		localdata[tile][index][i*2]=userData[i*2]; // copy x
		localdata[tile][index][i*2+1]=userData[i*2+1]; // copy y
		for(int j=0;j<4;j++)
			bb[tile][j]=(((j<2)?-1.0:1.0)*(bb[tile][j]-userData[i*2+j%2])<0)?userData[i*2+j%2]:bb[tile][j];
	}
	if(r+g+b>0)
	{// simply add color if it is set
		 Color[tile][index][0]=r; 
		 Color[tile][index][1]=g; 
		 Color[tile][index][2]=b; 
	}
}


void fadey_draw_particles(int index, float* particleArr, int N, int tile, double r=-1,  double g=-1, double b=-1)
{
	tileUsage[tile]=1;
	fetch_data(particleCloudNum, particleBounds, particlesSZ, particles, particlesColor, index, particleArr, N,tile, r, g, b);
}

void fadey_draw_1D(int index, float* graphArr, int N, int tile, double r=-1,  double g=-1, double b=-1)
{
	tileUsage[tile]=2;
	fetch_data(plotNum, plotBounds, plotsSZ, plots, plotsColor, index, graphArr, N,tile, r, g, b);
}

void fadey_close()
{
	if(inited[0]==1)
	{
		close_win[0]=1;
		inited[0]=0;
		pthread_mutex_lock(&close_win_mutex);
		pthread_mutex_destroy(&close_win_mutex);
		pthread_mutex_destroy(&data_update_mutex);
		printf("fadey draw: bye!\n");
	}
}

void	showMessage(GLfloat x, GLfloat y, char *message)
{
	#define FONTSZ 0.01
  glPushMatrix();
//  glDisable(GL_LIGHTING);
//  glTranslatef(x-FONTSZ*200.0/2.0, y-FONTSZ*200.0/2.0, z);
  glTranslatef(x, y, 0);
	float a=3;
	float b=3;
	float xg=0;
	int i=0;
	char c;
//  glScalef(FONTSZ, FONTSZ, FONTSZ);
//  for(i=0;i<sizeof(message);i++)
	while(*message)
	{
		xg=1.3*a*i;
		c=*message;
		glColor3f(0.5,0.7,0.5);
		glBegin(GL_LINES);
    //glutStrokeCharacter(GLUT_STROKE_ROMAN, *message);
		if(c=='1')
		{
				glVertex2f(xg+a/2, -b);
				glVertex2f(xg+a/2, 0);
				glVertex2f(xg+a/2, 0);
				glVertex2f(xg+a/2, b);
		}
		if(c=='2')
		{
				glVertex2f(xg+0, -b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, b);
				glVertex2f(xg+0, b);
				glVertex2f(xg+a, b);
		}
		if(c=='3')
		{
				glVertex2f(xg+0, -b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b);
				glVertex2f(xg+a, b);
				glVertex2f(xg+0, b);
		}
		if(c=='4')
		{
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b);
		}
		if(c=='5')
		{
				glVertex2f(xg+a, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b);
				glVertex2f(xg+a, b);
				glVertex2f(xg+0, b);
		}
		if(c=='6')
		{
				glVertex2f(xg+a, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b);
				glVertex2f(xg+a, b);
				glVertex2f(xg+0, b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, b);
		}
		if(c=='7')
		{
				glVertex2f(xg+0, -b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b);
		}
		if(c=='8')
		{
				glVertex2f(xg+a, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b);
				glVertex2f(xg+a, b);
				glVertex2f(xg+0, b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, 0);
		}
		if(c=='9')
		{
				glVertex2f(xg+a, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b);
				glVertex2f(xg+a, b);
				glVertex2f(xg+0, b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, 0);
		}
		if(c=='0')
		{
				glVertex2f(xg+a, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, -b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b);
				glVertex2f(xg+a, b);
				glVertex2f(xg+0, b);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, b);
				glVertex2f(xg+a, -b);
				glVertex2f(xg+a, 0);
		}
		if(c=='+')
		{
				glVertex2f(xg+a/2, -b/2);
				glVertex2f(xg+a/2, 0);
				glVertex2f(xg+a/2, 0);
				glVertex2f(xg+a/2, b/2);
				glVertex2f(xg+a/4, 0);
				glVertex2f(xg+a/2, 0);
				glVertex2f(xg+a/2, 0);
				glVertex2f(xg+3*a/4, 0);
		}
		if(c=='-')
		{
				glVertex2f(xg+a/4, 0);
				glVertex2f(xg+3*a/4, 0);
		}
		if((c=='.')||(c==','))
		{
				glVertex2f(xg+a/2+a/8, 3*b/4);
				glVertex2f(xg+a/2-a/8, 5*b/4);
		}
		if((c=='e')||(c=='E'))
		{
				glVertex2f(xg+a, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, 0);
				glVertex2f(xg+0, b/2);
				glVertex2f(xg+0, b/2);
				glVertex2f(xg+a, b/2);
				glVertex2f(xg+a, b);
				glVertex2f(xg+0, b);
				glVertex2f(xg+0, b/2);
				glVertex2f(xg+0, b);
				glVertex2f(xg+a, 0);
				glVertex2f(xg+a, b/2);
		}
		glEnd();
		message++;
		i++;
  }
//  glEnable(GL_LIGHTING);
  glPopMatrix();
}

