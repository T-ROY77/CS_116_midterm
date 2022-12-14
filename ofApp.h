//
//  RayCaster - Set of simple classes to create a camera/view setup for our Ray Tracer HW Project
//
//  I've included these classes as a mini-framework for our introductory ray tracer.
//  You are free to modify/change.   
//
//  These classes provide a simple render camera which can can return a ray starting from
//  it's position to a (u, v) coordinate on the view plane.
//
//  The view plane is where we can locate our photorealistic image we are rendering.
//  The field-of-view of the camera by moving it closer/further 
//  from the view plane.  The viewplane can be also resized.  When ray tracing an image, the aspect
//  ratio of the view plane should the be same as your image. So for example, the current view plane
//  default size is ( 6.0 width by 4.0 height ).   A 1200x800 pixel image would have the same
//  aspect ratio.
//
//  This is not a complete ray tracer - just a set of skelton classes to start.  The current
//  base scene object only stores a value for the diffuse/specular color of the object (defaut is gray).
//  at some point, we will want to replace this with a Material class that contains these (and other 
//  parameters)
//  
//  (c) Troy Perez - 17 October 2022
//
#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include <glm/gtx/intersect.hpp>

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	Ray() {}
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { cout << "SceneObject::intersect" << endl; return false; }
	virtual glm::vec3 getNormal(const glm::vec3& p) { return glm::vec3(0); }
	virtual glm::vec3 getIntersectionPoint() { return glm::vec3(1); }


	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 intersectionPoint;

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		bool intersect = (glm::intersectRaySphere(ray.p, glm::normalize(ray.d), position, radius, point, normal));
		setNormal(normal);
		intersectionPoint = point;
		return intersect;
	}
	void draw() {
		ofDrawSphere(position, radius);
	}
	void setNormal(const glm::vec3& p) { normal = p; }

	glm::vec3 getNormal(const glm::vec3& p) { return glm::normalize(normal); }

	glm::vec3 normal;

	float radius = 1.0;
};


class Light : public SceneObject {
public:
	Light(glm::vec3 p, float i) { position = p; intensity = i; }
	Light() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofSetColor(ofColor::gray);
		ofDrawSphere(position, radius);
	}
	void setIntensity(float i) {
		intensity = i;
	}
	float radius = 1.5;
	float intensity = 0.0;
};

class spotLight : public Light {
public:
	spotLight(glm::vec3 p, glm::vec3 aimPos, float i, float angle) {
		position = p; intensity = i; aimPoint = aimPos; direction = p - aimPoint;
		angle1 = angle;
		coneAngle = tan(glm::radians(angle)) * coneHeight;
	}
	spotLight() {}


	void draw() {
		ofSetColor(ofColor::blue);
		//ofDrawSphere(aimPoint, coneHeight/10);
		ofDrawSphere(aimPoint, coneAngle);




		// draw a cone object oriented towards aim position using the lookAt transformation
	// matrix.  The "up" vector is (0, 1, 0)
	//
		ofPushMatrix();
		glm::mat4 m = glm::lookAt(position, aimPoint, glm::vec3(0, 1, 0));
		ofMultMatrix(glm::inverse(m));
		ofRotate(-90, 1, 0, 0);
		ofSetColor(ofColor::lightGray);
		ofDrawCone(coneAngle, coneHeight);
		ofPopMatrix();
		ofDrawLine(position, aimPoint);

	}

	glm::vec3 direction = glm::vec3(0);
	glm::vec3 aimPoint = glm::vec3(0);

	float coneAngle = 0;
	float length = 20;
	float coneHeight = 50;
	bool lightSelected = false;
	bool aimPointSelected = false;

	float angle1 = 15;
};


//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { return false; }
	void draw() { }

};


//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen,
		float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);
	}
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
	}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	float sdf(const glm::vec3& p);
	glm::vec3 getNormal(const glm::vec3& p) { return this->normal; }
	glm::vec3 getIntersectionPoint() { return this->intersectionPoint; }
	void setIntersectionPoint(const glm::vec3& p) { intersectionPoint = p; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		plane.draw();
	}
	ofPlanePrimitive plane;
	glm::vec3 normal;
	float width = 20;
	float height = 20;
	glm::vec3 intersectionPoint;

};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}
	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 25);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};



class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void rayTrace();
	void drawGrid();
	void drawAxis(glm::vec3 position);
	ofColor ambient(ofColor diffuse);
	ofColor lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, Ray r, Light light);
	ofColor phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, const ofColor specular, float power, float distance, Ray r, Light light);
	ofColor shade(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, const ofColor specular, float power, Ray r);
	ofColor spotLightLambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, Ray r, spotLight light);
	ofColor spotLightLambert2(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, Ray r, spotLight light);

	void updateAngle(bool increase);

	glm::vec3 planeNormal;
	Plane p;
	Ray r;


	const float zero = 0.0;

	bool bHide = true;
	bool bShowImage = false;

	ofEasyCam  mainCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera sceneCam;
	ofCamera* theCam;    // set to current camera either mainCam or sideCam

	// set up one render camera to render image through
	//
	RenderCam renderCam;
	ofImage image;

	//object vectors
	//
	vector<SceneObject*> scene;
	vector<Light*> light;
	vector<spotLight*> spotLights;
	int lightIndex;

	vector<glm::vec3> aimPoint;
	vector<glm::vec3> spotLightPos;
	vector<float> angle;
	glm::vec3 mouseLast;


	int imageWidth = 1200;
	int imageHeight = 800;

	int closestIndex = 0;

	float slowdown = 1;


	//state variables
	//
	bool drawImage = false;
	bool trace = false;
	bool background = true;
	bool blocked = false;
	bool aimPointDrag = false;
	bool lightDrag = false;
	bool renderdraw = false;

	//GUI
	//
	ofxFloatSlider power;
	ofxFloatSlider intensity;
	ofxFloatSlider spotLightIntensity;

	ofxPanel gui;


};
