#include "stdafx.h"
#include "obj.h"
using namespace std;

obj::obj(float id) {
	name = id;
	xCord = 0;
	yCord = 0;
	zCord = 0;
	xAngle = 0;
	yAngle = 0;
	_isSelected = false;
	_isRightClick = false;
	
}

obj::~obj(){
}

float obj::getName() { 
	return name; 
}

vector<int> obj::getFaces() {
	return faces; 
}

float obj::getXAngle() {
	return xAngle; }

float obj::getYAngle() { 
	return yAngle;
}

void obj::selected(bool s) {
	_isSelected = s; 
}

bool obj::isSelected() {
	return _isSelected;
}

vector<float> obj::getCenter() {
	return center;
}

vector<float> obj::getColor()
{
	return colors;
}

void obj::addColor(float c)
{
	colors.push_back(c);
}

float obj::getX() {
	return xCord;
}

float obj::getY() { 
	return yCord;
}

float obj::getZ() { 
	return zCord; 
}

void obj::addFace(int eIndex) {
	faces.push_back(eIndex);
}
void obj::setCenter(float x, float y, float z) {
	center.clear();
	center.push_back(x);
	center.push_back(y);
	center.push_back(z);
}

void obj::updateAngels(float x, float y) {
	xAngle += x;
	yAngle += y;
	if (xAngle >= 360)
		xAngle -= 360;
	if (yAngle >= 360)
		yAngle -= 360;
}
 bool obj::operator == (const obj &a){
	 return(name== a.name);
}
 void obj::updateZ(float z) {
	 zCord += z;
 }
 void obj::updateLocation(float x, float y) {
	 xCord += x;
	 yCord += y;
 }
 void obj::setTexture(char *filename) {
	 strcpy(texture, filename);
 }
 char* obj::getTexture() {
	 return texture;
 }
 void obj::setRightClick(bool s){
	 _isRightClick = s;
 }
 bool obj::getRightClick() {
	 return _isRightClick;
 }
 void obj::printStatus() {
	 cout << "object number: " << name << " cord vector: <" << xCord << "," << yCord << "," << zCord <<">"<< endl;
	 cout << "angleX: " << xAngle << "   angelY: " << yAngle << endl;
	 cout <<"center vector: <" << center.at(0) << "," << center.at(1) << "," << center.at(2) << ">" << endl;
	 
 }