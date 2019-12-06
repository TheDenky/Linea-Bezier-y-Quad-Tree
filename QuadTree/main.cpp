#include "QuadTree.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <vector>
#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
using namespace std;

static int quadH = 512; 
static int quadW = 512;  
static int pencereUzunlugu = 512; 
static int pencereGenisligi = 712; 
static double ZOOM_P = 16.0; 
static double GEZ_P = 16.0;  


static float graphXMin = -18; 
static float graphXMax = 18;
float graphXRange = graphXMax - graphXMin;
float pixToXCoord = graphXRange / pencereGenisligi;

static float graphYMin = -18;
static float graphYMax = 18;
float graphYRange = graphYMax - graphYMin;
float pixToYCoord = graphYRange / pencereUzunlugu;


static bool solMouseDown = 0;
static bool sagMouseDown = 0;

vector <point> tumNoktalar; 
vector <point> bulunanNoktalar;  

point kareMerkezi(0, 0);
long double circleR = 64;
point kareGenisligi(64, 64);

point merkez(0, 0); 
point eksen(512, 512); 
static int kumeBoyutu = 1; 
QuadTree <int>* qtree;

long double randomNokta(); 


struct Button
{
	int   x;							
	int   y;							
	int   w;							
	int   h;											
	char* label;						
};
typedef struct Button Button;


Button Button1 = { 600,500, 200,70, "Random"};
Button Button2 = { 600,400, 200,70, "Temizle"};
Button Button3 = { 600,100, 200,70, "Yazdir" };
Button Button4 = { 600,200, 80,80, "+" };
Button Button5 = { 750,200, 80,80, "-" };


void ekranaYaz(void *font, char *text, int x, int y)
{
	glRasterPos2i(x, y);

	while (*text != '\0')
	{
		glutBitmapCharacter(font, *text);
		++text;
	}
}


void butonCiz(Button *b)
{
	int fontx;
	int fonty;

	if (b)
	{
		glColor3f(0.6f, 0.6f, 0.6f);

		glBegin(GL_QUADS);
		glVertex2i(b->x, b->y);
		glVertex2i(b->x, b->y + b->h);
		glVertex2i(b->x + b->w, b->y + b->h);
		glVertex2i(b->x + b->w, b->y);
		glEnd();

		glLineWidth(3);
		
		glColor3f(0.8f, 0.8f, 0.8f);

		glBegin(GL_LINE_STRIP);
		glVertex2i(b->x + b->w, b->y);
		glVertex2i(b->x, b->y);
		glVertex2i(b->x, b->y + b->h);
		glEnd();

		glColor3f(0.8f, 0.8f, 0.8f);
		
		glBegin(GL_LINE_STRIP);
		glVertex2i(b->x, b->y + b->h);
		glVertex2i(b->x + b->w, b->y + b->h);
		glVertex2i(b->x + b->w, b->y);
		glEnd();

		glLineWidth(1);


		const unsigned char *text =(unsigned char*) b->label;
		fontx = b->x + 10 + (glutBitmapLength(GLUT_BITMAP_HELVETICA_10, text)/2);
		fonty = b->y - 10 + (b->h + 10) / 2;

		glColor3f(1, 1, 1);
		ekranaYaz(GLUT_BITMAP_HELVETICA_10, b->label, fontx, fonty);
	}
}

int ButtonClickTest(Button* b, int x, int y)
{
	if (b)
	{
		if (x > b->x      &&
			x < b->x + b->w &&
			y > b->y      &&
			y < b->y + b->h) {
			return 1;
		}
	}

	return 0;
}



void  initializeViewMatrix()
{
	graphXRange = graphXMax - graphXMin;
	graphYRange = graphYMax - graphYMin;
	pixToXCoord = graphXRange / quadW;
	pixToYCoord = graphYRange / quadH;
}


static void resize(int w, int h)
{
	pencereGenisligi = w;
	pencereUzunlugu = h;
	initializeViewMatrix();
	glViewport(0, 0, (GLsizei)quadW, (GLsizei)quadH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(graphXMin, graphXMax, graphYMin, graphYMax);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void gez(double xAmount, double yAmount)
{
	graphXMin += xAmount;
	graphXMax += xAmount;
	graphYMin += yAmount;
	graphYMax += yAmount;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(graphXMin, graphXMax, graphYMin, graphYMax);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void zoom(double xAmount, double yAmount)
{
	graphXMin -= xAmount;
	graphXMax += xAmount;
	graphYMin -= yAmount;
	graphYMax += yAmount;

	initializeViewMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(graphXMin, graphXMax, graphYMin, graphYMax);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


long double randomNokta() {
	long double x = ((long double)rand() / (long double)RAND_MAX);
	return x;
}

static void noktalariBul()
{
	vector <pair <point, int> > bulunanlar;
	
	bulunanlar = qtree->bolgedekiObjeleriBul(
	{ kareMerkezi.x - kareGenisligi.x, kareMerkezi.y - kareGenisligi.y },
	{ kareMerkezi.x + kareGenisligi.x, kareMerkezi.y + kareGenisligi.y } );
	
	
	bulunanNoktalar.clear();
	bulunanNoktalar.resize(bulunanlar.size());

	for (int i = 0; i < bulunanlar.size(); ++i) {
		bulunanNoktalar[i] = bulunanlar[i].first;
	}
}

void renkDegistir(int index) {
	index %= 10;

	switch (index)
	{
	case 0:
		glColor3f(1, 0, 0);
		break;
	case 1:
		glColor3f(1, 1, 0);
		break;
	case 2:
		glColor3f(0, 1, 0);
		break;
	case 3:
		glColor3f(0, 1, 1);
		break;
	case 4:
		glColor3f(0, 0, 1);
		break;
	case 5:
		glColor3f(1, 0, 1);
		break;
	case 6:
		glColor3f(0.8, 0.2, 1);
		break;
	case 7:
		glColor3f(0.5, 0.5, 0.5);
		break;
	case 8:
		glColor3f(1, 0.5, 1);
		break;
	case 9:
		glColor3f(0.3, 0.2, 0.6);
		break;
	default:
		break;
	}


}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1, 1, 1);

	qtree->ciz();

	
	glColor3f (1, 1, 1);
	glPointSize (5.0);
	glBegin (GL_POINTS);
	for (unsigned i=0; i<tumNoktalar.size(); ++i){
	glVertex2f (tumNoktalar[i].x, tumNoktalar[i].y);
	}
	glEnd();
	
	
	glColor3f(0, 1, 0);
	glBegin(GL_LINE_LOOP);
	
	for (int ii = 0; ii < 100; ii++) {
		float theta = 2.0f * 3.1415926f * float(ii) / float(100);
		float x = circleR * cosf(theta);
		float y = circleR * sinf(theta);
		glVertex2f(x + kareMerkezi.x, y + kareMerkezi.y);
	}
	glEnd();

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for (unsigned i = 0; i<bulunanNoktalar.size(); ++i) {
		renkDegistir(i);
		glVertex2f(bulunanNoktalar[i].x, bulunanNoktalar[i].y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	glEnd();

	butonCiz(&Button1);
	butonCiz(&Button2);
	butonCiz(&Button3);
	ekranaYaz(GLUT_BITMAP_HELVETICA_10,"Quad tree UNSA", 600, 300);
	butonCiz(&Button4);
	butonCiz(&Button5);

	glFlush();
	glutSwapBuffers();
}


void selectionSort(vector<point>  toSort)
{
	for (vector<point>::size_type pos = 0; pos < toSort.size(); ++pos)
	{
		vector<point>::size_type min = pos;

		for (vector<point>::size_type i = pos + 1; i < toSort.size(); ++i)
		{
			if (toSort[i].x < toSort[min].x) {
				min = i;
			}else if (toSort[i].x == toSort[min].x) {
				if (toSort[i].y < toSort[min].y)
					min = i;
			}
		}

		if (min != pos)
		{
			swap(toSort[min].x, toSort[pos].x);
			swap(toSort[min].y, toSort[pos].y);
		}
	}

	cout << "Bulunan noktalar (" << toSort.size() << " adet) :\n";
	for (int i = 0; i < toSort.size(); i++) {
		cout << "x:" << toSort[i].x << " y:" << toSort[i].y << endl;
	}
	printf("\n");

}


void bulunanNoktalariYaz() {
	selectionSort(bulunanNoktalar);
}

static void key(unsigned char key, int x, int y)
{
	switch (key) {

	case 'h':
		if (circleR < 256) {
			circleR += 16;
			kareGenisligi = { circleR,circleR };
			noktalariBul();
			}
		break;
	case 'j':
		if (circleR > 32) {
			circleR -= 16;
			kareGenisligi = { circleR,circleR };
			noktalariBul();
			}
		break;
	case 'p':
	case 'P':
		bulunanNoktalariYaz();
		break;

	case 'a':
	case 'A':
		gez(-graphXRange / GEZ_P, 0);
		break;

	case 'd':
	case 'D':
		gez(graphXRange / GEZ_P, 0);
		break;

	case 'w':
	case 'W':
		gez(0, graphYRange / GEZ_P);
		break;

	case 's':
	case 'S':
		gez(0, -graphYRange / GEZ_P);
		break;

	case 'r':
	case 'R':
		for (int i = 0; i < 20; ++i) {
			point yeniNokta(eksen.x - (2 * eksen.x * randomNokta()),
				eksen.y - (2 * eksen.y * randomNokta()));
			tumNoktalar.push_back(yeniNokta);
			qtree->ekle(yeniNokta, 1);
		}
		break;
		
	case 'b':
		delete qtree;
		qtree = new QuadTree <int>(merkez, eksen, kumeBoyutu);
		tumNoktalar.clear();
		bulunanNoktalar.clear();
		break;

	case '+':
	case '=':
		zoom(graphXRange / ZOOM_P, graphYRange / ZOOM_P);
		break;

	case '-':
	case '_':
		zoom(-graphXRange / ZOOM_P, -graphYRange / ZOOM_P);
		break;

	case 'f':
		noktalariBul();
		break;
	}

	glutPostRedisplay();
}


static void mouse(int button, int state, int x, int y)
{
	point yeniNokta(x*pixToXCoord + graphXMin, -y*pixToYCoord + graphYMax);

	switch (button) {
	case GLUT_LEFT_BUTTON:
		switch (state) {
		case GLUT_DOWN:
			if (yeniNokta.x > -quadW && yeniNokta.x < quadW && yeniNokta.y < quadH && yeniNokta.y>-quadH) {
				solMouseDown = 1;
				tumNoktalar.push_back(yeniNokta);
				qtree->ekle(yeniNokta, 1);
			}
			else {

				if (ButtonClickTest(&Button1, yeniNokta.x, yeniNokta.y)) {
					for (int i = 0; i < 20; ++i) {
						point yeniNokta(eksen.x - (2 * eksen.x * randomNokta()),
							eksen.y - (2 * eksen.y * randomNokta()));
						tumNoktalar.push_back(yeniNokta);
						qtree->ekle(yeniNokta, 1);
					}
				}

				if (ButtonClickTest(&Button2, yeniNokta.x, yeniNokta.y)) {
					delete qtree;
					qtree = new QuadTree <int>(merkez, eksen, kumeBoyutu);
					tumNoktalar.clear();
					bulunanNoktalar.clear();
				}

				if (ButtonClickTest(&Button3, yeniNokta.x, yeniNokta.y)) {
					bulunanNoktalariYaz();
				}

				if (ButtonClickTest(&Button4, yeniNokta.x, yeniNokta.y)) {
					if (circleR < 256) {
						circleR += 16;
						kareGenisligi = { circleR,circleR };
						noktalariBul();
					}
				}
			
				if (ButtonClickTest(&Button5, yeniNokta.x, yeniNokta.y)) {
					if (circleR > 32) {
						circleR -= 16;
						kareGenisligi = { circleR,circleR };
						noktalariBul();
					}
				}
			}
			break;

		case GLUT_UP:
			solMouseDown = 0;
			break;
		}
		break;
	case GLUT_RIGHT_BUTTON:
		switch (state) {
		case GLUT_DOWN:
			if (yeniNokta.x > -quadW && yeniNokta.x < quadW && yeniNokta.y < quadH && yeniNokta.y>-quadH){
				sagMouseDown = 1;
				kareMerkezi = yeniNokta;
				noktalariBul();
			}
			break;

		case GLUT_UP:
			sagMouseDown = 0;
			break;
		}
		break;
	}
	glutPostRedisplay();
}


static void motion(int x, int y)
{
	point yeniNokta(x*pixToXCoord + graphXMin,
		-y*pixToYCoord + graphYMax);

	if (solMouseDown) {
		if (yeniNokta.x > -quadW && yeniNokta.x < quadW && yeniNokta.y < quadH && yeniNokta.y>-quadH) {
			tumNoktalar.push_back(yeniNokta);
			qtree->ekle(yeniNokta, 1);
		}
	}
	else if (sagMouseDown) {
		if (yeniNokta.x > -quadW && yeniNokta.x < quadW && yeniNokta.y < quadH && yeniNokta.y>-quadH) {
			kareMerkezi = yeniNokta;
			noktalariBul();
	     }
	}

	glutPostRedisplay();
}

void menu(int num) {
	switch (num)
	{
	case 0:
		for (int i = 0; i < 20; ++i) {
			point yeniNokta(eksen.x - (2 * eksen.x * randomNokta()),
				eksen.y - (2 * eksen.y * randomNokta()));
			tumNoktalar.push_back(yeniNokta);
			qtree->ekle(yeniNokta, 1);
		}
		break;
	case 1:
		delete qtree;
		qtree = new QuadTree <int>(merkez, eksen, kumeBoyutu);
		tumNoktalar.clear();
		bulunanNoktalar.clear();
		break;
	case 2:
		if (circleR < 256) {
			circleR += 16;
			kareGenisligi = { circleR,circleR };
			noktalariBul();
		}
		break;
	case 3:
		if (circleR > 32) {
			circleR -= 16;
			kareGenisligi = { circleR,circleR };
			noktalariBul();
		}
		break;
	case 4:
		bulunanNoktalariYaz();
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

/*
Menu olu�turur
*/
void menuOlustur() {
	glutCreateMenu(menu);
	glutAddMenuEntry("Random Deger Ata", 0);
	glutAddMenuEntry("Ekrani Temizle", 1);
	glutAddMenuEntry("Ruben Denky Valles Martinez", 2);
	glutAddMenuEntry("Ruben Denky Valles Martinez", 3);
	glutAddMenuEntry("The Denky", 4);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}


int main(int argc, char *argv[])
{
	srand(time(0));
	qtree = new QuadTree <int>(merkez, eksen, 1);
	glutInit(&argc, argv);
	glutInitWindowSize(pencereGenisligi, pencereUzunlugu);
	glutInitWindowPosition(10, 10);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("QuadTree ProLab Mert Eroglu");
	glutReshapeFunc(resize);
	menuOlustur();
	glutDisplayFunc(display);
	glutKeyboardFunc(key);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glClearColor(0, 0, 0, 0);
	for (int i = 0; i < 33; i++) {
		zoom(graphXRange / ZOOM_P, graphYRange / ZOOM_P);
	}
	glutMainLoop();

	
	
	return EXIT_SUCCESS;
}
