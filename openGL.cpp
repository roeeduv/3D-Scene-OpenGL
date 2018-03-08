#include "stdafx.h"
#pragma warning( disable : 4996 ) 

#include "bitmap_image.hpp"
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include "glut.h"
#include "Vector3f.h"
#include "obj.h"

#define BUFSIZE 512
#define screenWidth 512
#define screenHeight 512
#define maxSize 65535
using namespace std;
enum Mode {
	Camera, Scene, Object
};

GLuint selectBuf[BUFSIZE];
GLfloat vertices[maxSize * 3];
GLfloat normals[maxSize * 2 * 3];
GLuint faces[maxSize * 4 * 3];
GLuint faceElements[maxSize * 2];
float modelMatrix[16], projectionMatrix[16];
int vIndex, vnIndex, fIndex, eIndex;
Mode mode,objectMode;
vector<obj*> objVect;
obj* currentObject;

int mouseX, mouseY, objCounter, mirrorEffect;
bool leftMouseClicked, rightMouseClicked, blurEffect;
float xRotationUnits, yRotationUnits, translateForwardUnits;
float xRotationAngle, yRotationAngle;
float xCameraAngle, yCameraAngle;
float translateRightScene, objSelect;

void startPicking(int x, int y);
void processHits(int hits);
void resetPick();
void drawScene();
float checkAngle(float xCameraAngle);

void drawFloor(){
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glColor4f(0.4, 0, 0, 0.3);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	glVertex3f(-40, 0.0, 40);
	glVertex3f(40, 0.0, 40);
	glVertex3f(40, 0.0, -40);
	glVertex3f(-40, 0.0, -40);
	glEnd();
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

void printModel() //prints modelview matrix
{
	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
	printf("Modelview matrix: \n");
	for (int i = 0; i<4; i++)
	{
		for (int j = 0; j<4; j++)
			printf("%f ", modelMatrix[j * 4 + i]);
		printf("\n");
	}
}

void printProj() //prints projection matrix
{
	glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
	printf("projection matrix: \n");
	for (int i = 0; i<4; i++)
	{
		for (int j = 0; j<4; j++)
			printf("%f ", projectionMatrix[j * 4 + i]);
		printf("\n");
	}
}

void motionBlur(int val) {
		glAccum(GL_ACCUM, 0.1);
		glAccum(GL_RETURN, 1.0);
		glAccum(GL_MULT, 0.9);
		glFlush();
		glutTimerFunc(25000, motionBlur, 0);
}

float fixAngle(float x) {
	if (x >= 360)
		x -= 360;
	return x;
}

GLuint LoadTexture(const char * filename){
	GLuint texture;
	unsigned int width, height;
	unsigned char * data;
	FILE * file;
	file = fopen(filename, "rb");
	if (file == NULL) return 0;
	bitmap_image image(filename);
	width = image.height();
	height = image.width();
	data = (unsigned char *)malloc(width * height * 3);
	fread(data, width * height * 3, 1, file);
	fclose(file);
	for (unsigned int i = 0; i < width * height; ++i){
		int index = i * 3;
		unsigned char B, R;
		B = data[index];
		R = data[index + 2];
		data[index] = R;
		data[index + 2] = B;
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	free(data);
	
	return texture;
}

void resetPick() {
	if (objSelect != -1) {
		objVect.at(objSelect)->selected(false);
		objVect.at(objSelect)->setRightClick(false);
		objSelect = -1;
	}
}

void readFromFile(void) {
	FILE *f, *cTable;
	char c, d;
	f = fopen("doll.obj", "r");
	cTable = fopen("colorTable.csv", "r");
	int b, line ,counter;
	float col;
	char tex[30];
	vIndex = vnIndex = fIndex = eIndex = b = line = counter = 0;
	c = fgetc(f);
	while (c != EOF && vIndex < maxSize * 3 && vnIndex < maxSize * 2 * 3
		&& fIndex < maxSize * 4 * 3) {
		line++;
		switch (c) {
		case 'o':
		case 'g':
			if (objCounter > 0)
				objVect.push_back(currentObject);
			currentObject = new obj(objCounter++);
			d = fgetc(cTable);
			d = fgetc(cTable);
			counter = 2;
			for (int i = 0; i < 30; i++)
				tex[i] = 0;
			while (d != '\n' && d != EOF) { //Ignore Comments
			 if (d != ',') {
				 if (counter == 21) {
					 tex[0] = d;
					 fscanf(cTable, "%s", &tex[1]);
				 }
				 else {
					 fscanf(cTable, "%f", &col);
					 currentObject->addColor(col);
				 }
				 }
				d = fgetc(cTable);
				counter++;
			}
			currentObject->setTexture(tex);
			break;
			while (c != '\n' && c != EOF)
				c = fgetc(f);
			break;
		case 'v':
			c = fgetc(f);
			if (c == 'n') {
				fscanf(f, " %f %f %f\n", &normals[vnIndex],
					&normals[vnIndex + 1], &normals[vnIndex + 2]);
				vnIndex += 3;
			}
			else {
				fscanf(f, "%f %f %f\n", &vertices[vIndex],
					&vertices[vIndex + 1], &vertices[vIndex + 2]);
				vIndex += 3;
			}
			break;
		case 'f':
			c = fgetc(f);
			b = 2;
			while (c != '\n' && b > 0) {
				b = fscanf(f, "%d//%d", &faces[fIndex], &faces[fIndex + 1]);
				if (b == 2) {
					fIndex += 2;
					c = fgetc(f);
					faceElements[eIndex]++;
				}
			}
			currentObject->addFace(eIndex);
			eIndex++;
			break;
		default:
			while (c != '\n' && c != EOF) //Ignore Comments
				c = fgetc(f);
			break;
		}
		if (c != EOF)
			c = fgetc(f);
	}
	objVect.push_back(currentObject);
	fclose(f);
	fclose(cTable);
}

void COM() {
	//this function calculate the Center Of Mass of an object
	int i = 0, x, y, z, counter;
	vector<int> facesVec;
	vector<obj*>::iterator it;
	//iterate over all objects
	for (it = objVect.begin(); it != objVect.end(); ++it) {
		x = y = z = counter = 0;
		facesVec = (*it)->getFaces();
		vector<int>::iterator it2;
		//sum each cordinate 
		for (it2 = facesVec.begin(); it2 != facesVec.end(); ++it2) {
			for (unsigned int j = 0; j < faceElements[*it2] * 2; j += 2) {
				x += vertices[(faces[i + j] - 1) * 3];
				y += vertices[(faces[i + j] - 1) * 3 + 1];
				z += vertices[(faces[i + j] - 1) * 3 + 2];
				counter++;
			}
			i += faceElements[*it2] * 2;
		} 
		//get average for each cordinate and update center of the object
		x = x / counter;
		y = y / counter;
		z = z / counter;
		(*it)->setCenter(x, y, z);
	}
}

void init() {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 2, 500);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, -200, 0, 0, 0, 0, 1, 0);
	
	//init light
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat light_direction[] = { 0,-1.0,0 };
	GLfloat light_ambient[] = { 0.5, 0.5, 0.5 };
	GLfloat light_diffuse[] = { 0.0, 0.5, 0.5 };
	GLfloat light_specular[] = { 0.0, 0.0, 0.5 };
	GLfloat mat_shin[] = { 5.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_direction);
	glLightfv(GL_LIGHT0, GL_SHININESS, mat_shin);
	//init all global variables
	leftMouseClicked = false;
	rightMouseClicked = false;
	mode = Camera;
	xCameraAngle = yCameraAngle  = translateRightScene = 
    mouseX = mouseY = objCounter = xRotationAngle = yRotationAngle = 0;
	xRotationUnits = 180.0 / (float)screenWidth;
	yRotationUnits = 180.0 / (float)screenHeight;
	translateForwardUnits = 5.0 / (float)screenHeight;
	objSelect = -1;
	objectMode = Camera;
	readFromFile();
	COM();
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (mirrorEffect){
		glClear( GL_STENCIL_BUFFER_BIT);
		// Don't update depth. 
		glDisable(GL_DEPTH_TEST);
		//make mirror red/ black
	//	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		// Draw 1 into the stencil buffer and draw floor
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
		drawFloor();

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		/* Now, only render where stencil is set to 1. */
		glStencilFunc(GL_EQUAL, 1, 0xffffffff);  /* draw if ==1 */
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glPushMatrix();
		//draw reflection
		glScalef(1.0, -1.0, 1.0);
		drawScene();
		glPopMatrix();
		glDisable(GL_STENCIL_TEST);
	}
	//draw scene
	drawScene();
}

void drawScene(){
	glEnable(GL_COLOR_MATERIAL);
	glBegin(GL_LINES);
	//X Axis = red
	glColor3f(1, 0, 0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1000.0, 0.0, 0.0);
	//Y Axix= green
	glColor3f(0, 1, 0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1000.0, 0.0);
	//Z Axis= blue
	glColor3f(0, 0, 1);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1000.0);
	glEnd();
	glDisable(GL_COLOR_MATERIAL);

	int i = 0;
	vector<int> faces_vect;
	vector<obj*>::iterator it;
	//iterate all objects
	for (it = objVect.begin(); it != objVect.end(); ++it) {
		//define ambient,diffuse,spec and initial it
		GLfloat object_ambient[3];
		GLfloat object_diffuse[3];
		GLfloat object_specular[3];
		vector<float> objColor = (*it)->getColor();
		for (int j = 0; j < 3; j++) {
			object_ambient[j] = objColor.at(j);
			object_diffuse[j] = objColor.at(j + 3);
			object_specular[j] = objColor.at(j + 6);
		}
		faces_vect = (*it)->getFaces();
			glLoadName((*it)->getName());
		
		glPushMatrix();
		//Save current matrix & Load Identity
		float mview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mview);
		glLoadIdentity();
		//Translate the object
		glTranslatef((*it)->getX(), (*it)->getY(), (*it)->getZ());
		//Multiply by saved matrix
		glMultMatrixf(mview);
		//Rotate the object
		vector<float> center = (*it)->getCenter();
		glTranslatef(center.at(0), center.at(1), center.at(2));
		glRotatef((*it)->getXAngle(), 1.0, 0.0, 0.0);
		glRotatef((*it)->getYAngle(), 0.0, 1.0, 0.0);
		//translate to Center of Masa of object
		glTranslatef(center.at(0)*-1, center.at(1)*-1, center.at(2)*-1); 
		if ((*it)->isSelected()) {
			glEnable(GL_BLEND);
			glEnable(GL_COLOR_MATERIAL);
			glColor4f(1.0, 0, 0, 0.7);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glutSolidSphere(3, 20, 20);
			glDisable(GL_COLOR_MATERIAL);
			glDisable(GL_BLEND);
		}
		GLuint texture;
		texture = LoadTexture((*it)->getTexture());
		if (texture)
			glEnable(GL_TEXTURE_2D);
		//set color valus;
		glMaterialfv(GL_FRONT, GL_AMBIENT, object_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, object_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, object_specular);
		//actual drawing of faces
		vector<int>::iterator it2;
		for (it2 = faces_vect.begin(); it2 != faces_vect.end(); ++it2) {
			glBegin(GL_POLYGON);
			for (unsigned int j = 0; j < faceElements[*it2] * 2; j += 2) {
				glNormal3f(normals[(faces[i + j + 1] - 1) * 3],
					normals[(faces[i + j + 1] - 1) * 3 + 1],
					normals[(faces[i + j + 1] - 1) * 3 + 2]);
				if (j == 0)
					glTexCoord2f(0.0, 0.0);
				else if (j == 2)
					glTexCoord2f(0.0, 1.0);
				else if (j == 4)
					glTexCoord2f(1.0, 0.0);
				else if (j == 6)
					glTexCoord2f(1.0, 1.0);
				glVertex3f(vertices[(faces[i + j] - 1) * 3],
					vertices[(faces[i + j] - 1) * 3 + 1],
					vertices[(faces[i + j] - 1) * 3 + 2]);
			}
			glEnd();
			i += faceElements[*it2] * 2;
		}
		
		if ((*it)->isSelected() && (*it)->getRightClick())
			motionBlur(1000); 
		
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	}
}

void display(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glPushMatrix(); //save current state
	glRotatef(xRotationAngle, 0.0, 1.0, 0.0); //rotation matrix
	glRotatef(yRotationAngle, 1.0, 0.0, 0.0);//rotation matrix
	draw(); //draw image
	glPopMatrix(); //go to previous state
	glFlush();
}

void currMode(int v) {
	//current window name is current mode 
	string windowName = "Current mode - ";
	if (mode == Camera)
		windowName += "CAMERA";
	else if (mode == Scene)
		windowName += "SCENE";
	else 
		windowName += "OBJECT";

	glutSetWindowTitle(windowName.c_str()); //update window name
	glutPostRedisplay(); //save changes
	glutTimerFunc(1, currMode, 0);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'c':
		mode = Camera;
		resetPick();
		break;
	case 's':
		mode = Scene;
		resetPick();
		break;
	case 'o':
			mode = Object;
			objectMode = Camera;
			break;
	case '1':
		if (mode == Object)
			objectMode = Camera;
		break;
	case '2':
		if (mode == Object)
			objectMode = Scene;
		break;
	case '3':
		if (mode == Object)
			objectMode = Object;
		break;
	default:
		break;
	}
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN)
			leftMouseClicked = true;
		else
			leftMouseClicked = false;
	}
	else  if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN)
			rightMouseClicked = true;
		else
			rightMouseClicked = false;
	}
	if (mode == Object)
		startPicking(x, y);
	//update current mouse location
	mouseX = x;
	mouseY = y;
}

void startPicking(int x, int y) {
	int hits;
	GLint viewport[4];
	glSelectBuffer(BUFSIZE, selectBuf);
	glRenderMode(GL_SELECT); //select object mode (for draw function)
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	glInitNames(); //inital all object names
	glPushName(0);//start from first obj
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(x, viewport[3] - y,1, 1, viewport);
	gluPerspective(60, 1, 1, 500);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); //Add Global&Camera modes chaings
	glTranslatef(translateRightScene, 0, 0);
	glRotatef(xRotationAngle, 0.0, 1.0, 0.0);
	glRotatef(yRotationAngle, 1.0, 0.0, 0.0);
	draw();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();
	hits = glRenderMode(GL_RENDER);
		processHits(hits);
	glMatrixMode(GL_MODELVIEW);
}

void processHits(int hits) {
	unsigned int minZ, *ptr, names, *ptrNames;
	ptrNames = 0;

	ptr = (unsigned int*)selectBuf;
	minZ = 0xffffffff;
	for (int i = 0; i < hits; i++) {
		names = *ptr;
		ptr++;
		if (*ptr < minZ) {
			minZ = *ptr;
			ptrNames = ptr + 2;
		}
		ptr += names + 2; //Go for the next hit
	}
	if (leftMouseClicked) {
		if (hits == 0) {
			cout << "clicked outside of any object " << endl;
			resetPick();
		}
		else if (objSelect == -1) {
			//no object is selected
			objVect.at(*ptrNames)->selected(true);
			objSelect = *ptrNames;
			cout << "object " << *ptrNames << " is selected" << endl;
			}
		else if (objSelect == *ptrNames) {
			//same object
			cout << "object " << *ptrNames << " allready selected" << endl;
		}
		else {
			//replace pick object
			cout << "object " << objSelect << " replaced with object " << *ptrNames << endl;
			resetPick();
			objSelect = *ptrNames;
			objVect.at(objSelect)->selected(true);
		}
	}
}

void motion(int x, int y) {
	int differenceX = mouseX - x;
	int differenceY = mouseY - y;
	float angleX =  float(mouseX - x) * xRotationUnits;
	float angleY =  float(mouseY - y) * yRotationUnits;

	if (mode == Camera) {
		if (leftMouseClicked) {
			//rotate camera around its axies
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			glLoadIdentity();
			xCameraAngle += angleX;
			yCameraAngle += angleY;
			xCameraAngle = fixAngle(xCameraAngle);
			yCameraAngle = fixAngle(yCameraAngle);
			glRotatef(angleX, 0.0, 1.0, 0.0);
			glRotatef(angleY, 1.0, 0.0, 0.0);
			glMultMatrixf(mview);
		}
		else if (rightMouseClicked) {
			//translation along the x or y axes
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			glLoadIdentity();
			glTranslatef(differenceX * translateForwardUnits * 5, -1 * differenceY * translateForwardUnits * 5, 0);
			glMultMatrixf(mview);
		}
		else {
			//moving forward or backward
			float mview[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mview);
			glLoadIdentity();
			glTranslatef(0, 0, -1*differenceY * translateForwardUnits*5);
			glMultMatrixf(mview);
		}
	}

	else if (mode == Scene) {
		if (leftMouseClicked) {
			//rotate scene around its axies
			xRotationAngle += angleX;
			yRotationAngle += angleY;
			xRotationAngle = fixAngle(xRotationAngle);
			yRotationAngle = fixAngle(yRotationAngle);
		}
		else {
			vector<obj*>::iterator it;
			for (it = objVect.begin(); it != objVect.end(); ++it) {
				if (rightMouseClicked) {
					//middle mouse is pressed
					//transalte origin toward X and Y axies
					(*it)->updateLocation(-1 * differenceX * translateForwardUnits * 5, differenceY * translateForwardUnits * 5);
				}
				else {
					//transalte origin toward Z axis	
					(*it)->updateZ(differenceY * translateForwardUnits * 5);
				}
			}
		}
	}
	else if (mode == Object) {
		if (leftMouseClicked && objSelect > -1) {	
			objVect.at(objSelect)->setRightClick(false); 
			if (objectMode == Camera)
				//rotate according to camera axis
				objVect.at(objSelect)->updateAngels(yCameraAngle + angleY, xCameraAngle + angleX);
			if (objectMode == Scene)
				//rotate according to scene axis
				objVect.at(objSelect)->updateAngels(yRotationAngle + differenceY, xRotationAngle + differenceX);
			if (objectMode == Object) 
				//rotate according to object axis
				objVect.at(objSelect)->updateAngels(angleY, angleX);
		}
		else if (rightMouseClicked && objSelect > -1) {
			objVect.at(objSelect)->setRightClick(blurEffect);
			if (objectMode == Camera) {
				objVect.at(objSelect)->updateLocation(5 * -1 * differenceX * translateForwardUnits, 5 * differenceY * translateForwardUnits);
			}
			else	if (objectMode == Scene)
				objVect.at(objSelect)->updateLocation(-differenceX , differenceY);
			else if (objectMode == Object)
				objVect.at(objSelect)->updateLocation(-1 * differenceX * translateForwardUnits, differenceY * translateForwardUnits);	
		}
	}
	
	mouseX = x;
	mouseY = y;
}

int main(int argc, char **argv) {
	mirrorEffect = blurEffect = 0;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-m")==0)
			mirrorEffect = 1;
		if (strcmp(argv[i], "-b")==0)
			blurEffect = 1;
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(2, currMode, 1);
	glutMainLoop();
	return 0;
}