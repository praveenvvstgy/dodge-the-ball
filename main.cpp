#include <stdlib.h>
#include <time.h>
#include <glut.h>
#include <cstdio>
#include <math.h>
#include <windows.h>
#include "vmath.h"

// Number of enemy balls in the screen currently
int max_enemy_balls = 1;
// Maximum number of enemy balls that can be introduced
#define ENEMY_BALLS 20
// Size of the widow
static double wWidth=800,wHeight=600;
// timer
int timer = 0;
//  The number of frames
int frameCount = 0;
//  Number of frames per second
float fps = 0;
//  currentTime - previousTime is the time elapsed
//  between every call of the Idle function
int currentTime = 0, previousTime = 0;


void init();
//orange, yellow, light blue, dark blue, purple
// pink, dark gray
float color[][3] = { {1.0,.58,.21}, {1.0,.79,.28}, {.18, .67, .84}, {0,.49,.96}, {.35,.35,.81},
					{1,.17,.34}, {.56,.56,.58}
};


// Make sures that the window cannot be resized by the user to take advantage
static void resize(int width, int height)
{
	glutReshapeWindow(wWidth,wHeight);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-wWidth/2,wWidth/2, -wHeight/2,wHeight/2, -50.0, 50.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity() ;
}

// Returns the distance between two points
double distance(double x1, double y1, double x2, double y2)
{
    return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}


//Radius and Centers of balls................also the direction (sign) of velocity and magnitude of velocity
static double rad=25.0,x[ENEMY_BALLS + 1],y[ENEMY_BALLS + 1],sx[ENEMY_BALLS + 1],sy[ENEMY_BALLS + 1],vx[ENEMY_BALLS + 1],vy[ENEMY_BALLS + 1];
int startTime,endTime;  //calculating the time
bool start=false;   //true when game starts
bool hitWall=false,checkBound=true; 
bool gameover = false;
//hitWall for boundary checking and reflection along wall.............and checkBound becomes false when game is over
void collision2Ds(double& xx1, double& yy1, double& xx2, double& yy2,
                 double& vxx1, double& vyy1, double& vxx2, double& vyy2,
                 double& sxx1, double& syy1, double& sxx2, double& syy2)     
{
       Vector2<double> p1(xx1,yy1);//centre of ball 1
       Vector2<double> p2(xx2,yy2);//circle of ball 2
       Vector2<double> v1(sxx1*vxx1,syy1*vyy1);//velocity of ball 1
       Vector2<double> v2(sxx2*vxx2,syy2*vyy2);//velocity of ball 2
        Vector2<double> posDiff = (p2-p1);//collision line vector
        Vector2<double> velDiff = (v2-v1);

        Vector2<double> posDiffNorm=posDiff;
        posDiffNorm.normalize();//normalized collision line or unit normal line
        Vector2<double> tangent;//unit tangent between two balls
        tangent.x=-posDiffNorm.y;
        tangent.y=posDiffNorm.x;
        //redundant code for understanding the process
        Vector2<double> v1n=v1*posDiffNorm;//components of velocity of balls along normal
        Vector2<double> v2n=v2*posDiffNorm;
        Vector2<double> v1t=v1*tangent;//components of velocity of balls along tangent
        Vector2<double> v2t=v2*tangent;
        //swapping the normal components and keeping the tangential components same
        Vector2<double> newv1n=v2n*posDiffNorm;
        Vector2<double> newv2n=v1n*posDiffNorm;
        Vector2<double> newv1t=v1t*tangent;
        Vector2<double> newv2t=v2t*tangent;

        //assign new velocities
            v1=newv1n+newv1t;
            v2=newv2n+newv2t;


        //adjust the ball position to avoid sticking of balls in case of intersection of ball boundaries
            Vector2<double> adjust=posDiffNorm*(2*rad-distance(xx1,yy1,xx2,yy2))/2;
            if(newv1n<0)p1+=adjust;
            else p1-=adjust;
            if(newv2n<0)
            p2-=adjust;
            else p2+=adjust;
        //assign the changed positions
        xx1=p1.x;
        xx2=p2.x;
        yy1=p1.y;
        yy2=p2.y;
        //change the sign of velocity accordingly
        if(sxx1*v1.x*vxx1<0)
        sxx1=-sxx1;
        if(syy1*v1.y*vyy1<0)
        syy1=-syy1;
        if(sxx2*v2.x*vxx2<0)
        sxx2=-sxx2;
        if(syy2*v2.y*vyy2<0)
        syy2=-syy2;

        //keep the absolute values in velocities
        vxx1=fabs(v1.x);
        vxx2=fabs(v2.x);
        vyy1=fabs(v1.y);
        vyy2=fabs(v2.y);

       return;
}
// Maintain velocity above a certain desired value
void checkVel(double &vx,double &vy)
{
    if(checkBound==true)
    {
		if(vx<2)
		vx=2;
		if(vy<2)
		vy=2;
    }
}
//code for checking boundary collisions
void checkBoundary()
{
    if(checkBound==true)
    {
     for(int i=1;i<max_enemy_balls + 1;i++)
     {
        if(x[i]>=wWidth/2-rad)
        {
                x[i]=wWidth/2-rad;
                hitWall=true;
        }
		if(x[i]<=rad-wWidth/2)
        {
                x[i]=rad-wWidth/2;
                hitWall=true;
        }


        if(y[i]>=wHeight/2-rad)
        {
            y[i]=wHeight/2-rad;
                hitWall=true;
        }
        if(y[i]<=rad-wHeight/2)
        {y[i]=rad-wHeight/2;
            hitWall=true;
        }
    }

    }
}


//When game is over drop all the balls
static void dropAll()
{
		int i;
		if(checkBound)
			printf("%d",endTime-startTime);
        checkBound=false;
		//start = false;
        endTime=glutGet(GLUT_ELAPSED_TIME);
		for(i=1;i<=max_enemy_balls;i++)
		{
			vx[i]=0;
			vy[i]=-1.5*sy[i];
		}
        glutPostRedisplay();
}
//randomize the initial positions.............unsatisfactory code
void randomInit()
{
		int i;

		for(i=1;i<=max_enemy_balls;i++)   
		{
			srand(time(0));
			sx[i]=sy[i]=1;
			x[i]=rand()%800-400;
			y[i]=rand()%600-300;
		}

		for(i=1;i<=max_enemy_balls;i++)       
		{	
			srand(time(0));
			vx[i]=(double)(rand()%10)/5;
			vy[i]=(double)(rand()%10)/5;
		}

}
//check the collisions for all combinations of balls
void checkCollision()
{		
		int i,j;
		for(i=1;i<max_enemy_balls;i++)
		{
			for(j=i+1;j<=max_enemy_balls;j++)
			{
				if(distance(x[i],y[i],x[j],y[j])<=2*rad)
				{
						collision2Ds(x[i],y[i],x[j],y[j],vx[i],vy[i],vx[j],vy[j],sx[i],sy[i],sx[j],sy[j]);

				}
			}
		}

		for(i=1;i<=max_enemy_balls;i++)
		{
			if(distance(x[i],y[i],x[0],-y[0])<=2*rad)
			{
				dropAll();
			}
		}
        glutPostRedisplay();

}

void deinit()
{
	glDisable(GL_LIGHT0);
    glDisable(GL_NORMALIZE);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
}



static void display(void)
{
	int i;
	char timerarray[20], fpsarray[20];
    glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(.17,0.17,0.17,1);

	

    glColor3d(0.27,0.85,0.46);
    // The ball controlled by the user
    glPushMatrix();
        glTranslated(x[0],-y[0],0.0);
        glutSolidSphere(rad,30,30);
    glPopMatrix();

    // Enemy balls
	
	for(i=1;i<=max_enemy_balls;i++)
	{
		glColor3fv(color[i%6]);
		glPushMatrix();
        glTranslated(x[i],y[i],0.0);
        glutSolidSphere(rad,30,30);
		glPopMatrix();
	}
	
	//display timer
	deinit();
	sprintf(timerarray,"Score: %d",timer);
	glColor3f(.18,.67,.84);
	glRasterPos2f(-390,-290);
	for(i = 0; timerarray[i]!='\0';i++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, timerarray[i]);
	//display fps
	sprintf(fpsarray,"FPS: %4.2f",fps);
	glRasterPos2f(300,-290);
	for(i = 0; fpsarray[i]!='\0';i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, fpsarray[i]);

	init();

    checkCollision();

    checkBoundary();

	//try to maintain velocity of balls
    for(int i=1;i<=max_enemy_balls + 1;i++)
		checkVel(vx[i],vy[i]);
    
	if(hitWall==true)
    {
			for(int i=1;i<max_enemy_balls + 1;i++)
			{
				if(x[i]>=wWidth/2-rad||x[i]<=rad-wWidth/2)
				sx[i]=-sx[i];
				if(y[i]>=wHeight/2-rad||y[i]<=rad-wHeight/2)
				sy[i]=-sy[i];
			}
            hitWall=false;
    }
    //if game was started the change the position
	if(start){
		for(int i=1;i<max_enemy_balls + 1;i++)
		{
			x[i]=x[i]+vx[i]*sx[i];
			y[i]=y[i]+vy[i]*sy[i];
		}
    }
	
    glFlush();
    glutSwapBuffers();
    glGetError();
	if(fps > 300)
	Sleep(2);
}


void addball(int no)
{
	double xpos,ypos;
	if(checkBound) 
	{
		timer += 1;
		if(max_enemy_balls < ENEMY_BALLS && timer%4 == 0 && timer != 0)
		{
			srand(time(0));
			xpos = rand()%800-400;
			ypos = rand()%600-300;
			while(distance(xpos,ypos,x[0],-y[0])<=2*rad)
			{
				srand(max_enemy_balls);
				xpos = rand()%800-400;
				ypos = rand()%600-300;
			}
			max_enemy_balls += 1;
			sx[max_enemy_balls]=sy[max_enemy_balls]=1;
			x[max_enemy_balls]=xpos;
			y[max_enemy_balls]=ypos;
			vx[max_enemy_balls]=(double)(rand()%15)/5;
			vy[max_enemy_balls]=(double)(rand()%15)/5;
			//glutTimerFunc(1000,addball,1);
		}
	}
	glutTimerFunc(1000,addball,1);
}


//To quit the game press q........to start press s
void keyboard (unsigned char key, int x, int y)
{
        switch(key)
        {
                case 'q':
                    exit(0);
                    break;
                case 's':
                    start=true;
					glutTimerFunc(1000,addball,1);
                    if(start==false)
                    startTime=glutGet(GLUT_ELAPSED_TIME);//note down the initial time
                    break;
				 case 'r':
                    start=true;
					checkBound = true;
					timer = 0;
					randomInit();
					max_enemy_balls = 1;
					if(start==false)
                    startTime=glutGet(GLUT_ELAPSED_TIME);//note down the initial time
                    break;
				default:
                    break;
        }
}
//Function to control your ball with mouse
void mouse (int x2,int y2)
{

        x[0]=x2-wWidth/2;
        y[0]=y2-wHeight/2;
        glutPostRedisplay();


}

void reverse(int button, int state, int x, int y) {
    if(state == GLUT_DOWN) {
        for(int i=1;i<max_enemy_balls + 1;i++)
		{
			sx[i]=-sx[i];
			sy[i]=-sy[i];
		}
    }
}

void calculateFPS()
{
    //  Increase frame count
    frameCount++;

    //  Get the number of milliseconds since glutInit called 
    //  (or first call to glutGet(GLUT ELAPSED TIME)).
    currentTime = glutGet(GLUT_ELAPSED_TIME);

    //  Calculate time passed
    int timeInterval = currentTime - previousTime;

    if(timeInterval > 1000)
    {
        //  calculate the number of frames per second
        fps = frameCount / (timeInterval / 1000.0f);

        //  Set time
        previousTime = currentTime;

        //  Reset frame count
        frameCount = 0;
    }
}

static void idle(void)
{
	//  Calculate FPS
    calculateFPS();

    glutPostRedisplay();
}

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void init()
{
	glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
}
/* Program entry point */

int main(int argc, char *argv[])
{

    glutInit(&argc, argv);
    glutInitWindowSize(wWidth,wHeight);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

    glutCreateWindow("Dodge The Ball");
    randomInit();

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutPassiveMotionFunc(mouse);
	glutMotionFunc(mouse);
    glutKeyboardFunc(keyboard);
	glutMouseFunc(reverse);
    glutIdleFunc(idle);
	
	init();


    glutMainLoop();
	
    return EXIT_SUCCESS;
}


