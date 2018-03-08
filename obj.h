#pragma once
#include "stdafx.h"
#include <vector>
#include <stdio.h>
#include <iostream>

using namespace std;
class obj {
private:
	float name;
	float xCord, yCord, zCord;
	float xAngle, yAngle;
	vector<float> colors; //need to add functions for color
	vector<int> faces;
	vector<float> center;
	bool _isSelected;
	bool _isRightClick;
	char texture[30];
	
public:
	obj(float);
	~obj();
	float getName();
	float getX() ;
	float getY() ;
	float getZ();
	vector<int> getFaces() ;
	vector<float> getCenter();
	vector<float> getColor();
	void addColor(float c);
	float getXAngle() ;
	float getYAngle();
	void selected(bool s) ;
	bool isSelected() ;
	void addFace(int);
	void setCenter(float, float, float);
	void updateAngels(float, float);
	void obj::updateZ(float z);
	void obj::updateLocation(float x, float y);
	bool operator==(const obj & a);
	char* obj::getTexture();
	void obj::setTexture(char *filename);
	void obj::printStatus();
	void obj::setRightClick(bool s);
	bool obj::getRightClick();

};