#include "ofApp.h"

#include <glm/gtx/intersect.hpp>

//calculate cone width with angle

//make manipulator more smooth

//change phong and lambert to only light in cone width



// Intersect Ray with Plane  (wrapper on glm::intersect*
//

bool Plane::intersect(const Ray& ray, glm::vec3& point, glm::vec3& normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		setIntersectionPoint(r.evalPoint(dist));

		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width
			/ 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z +
			height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1]
			&& point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}

// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}


//--------------------------------------------------------------
void ofApp::setup() {
	image.allocate(imageWidth, imageHeight, ofImageType::OF_IMAGE_COLOR);

	gui.setup();
	gui.add(intensity.setup("Light intensity", .2, .05, 5));
	gui.add(power.setup("Phong p", 100, 10, 10000));
	bHide = true;

	theCam = &mainCam;

	mainCam.setPosition(glm::vec3(0, 350, 400));
	mainCam.setTarget(glm::vec3(0, 0, 0));
	sideCam.setPosition(glm::vec3(5, 0, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(.1);
	previewCam.setFov(90);
	previewCam.setPosition(renderCam.position);
	previewCam.lookAt(glm::vec3(0, 0, -1));
	sceneCam.setPosition(glm::vec3(0, 50, 100));
	sceneCam.lookAt(glm::vec3(0, 0, 0));







	cout << "h to toggle GUI" << endl;
	cout << "c to toggle camera mode" << endl;
	cout << "t to start ray tracer" << endl;
	cout << "d to show render" << endl;


	aimPoint.clear();
	spotLightPos.clear();
	angle.clear();

	aimPoint.push_back(glm::vec3(1, -5, 0));
	spotLightPos.push_back(glm::vec3(-20, 30, 45));
	angle.push_back(15);


	//aimPoint.push_back(glm::vec3(3, 3, 0));
	//spotLightPos.push_back(glm::vec3(-30, 30, 45));

	

}

void ofApp::updateAngle(bool increase) {
	if (increase) {
		if (!mainCam.getMouseInputEnabled()) {
			for (int i = 0; i < spotLights.size(); i++) {
				 if (angle[i] < 50) angle[i] += .5; 
			}
		}
	}
	else {
		if (!mainCam.getMouseInputEnabled()) {
			for (int i = 0; i < spotLights.size(); i++) {
				if (angle[i] > 10) angle[i] -= .5;
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::update() {
	//p.intersect(r, aimPoint[lightIndex], glm::normalize(mainCam.getPosition() - aimPoint[lightIndex]));

}

//--------------------------------------------------------------
void ofApp::rayTrace() {

	cout << "drawing..." << endl;

	ofColor closest;
	float distance = FLT_MIN;
	float close = FLT_MAX;
	closestIndex = 0;
	for (int i = 0; i < image.getWidth(); i++) {
		for (int j = 0; j < image.getHeight(); j++) {
			background = true;																//reset variables every pixel
			distance = FLT_MIN;
			close = FLT_MAX;
			closestIndex = 0;

			float u = (i + .5) / image.getWidth();
			float v = 1 - (j + .5) / image.getHeight();

			Ray r = renderCam.getRay(u, v);
			for (int k = 0; k < scene.size(); k++) {
				if (scene[k]->intersect(r, scene[k]->intersectionPoint, glm::vec3(0, 1, 0))) {
					background = false;														//if intersected with scene object, pixel is not background

					distance = glm::distance(r.p, scene[k]->position);						//calculate distance of intersection
					if (distance < close)													//if current object is closest to viewplane
					{
						closestIndex = k;													//save index of closest object
						close = distance;													//set threshold to new closest distance
					}
				}
			}
			if (!background) {
				//add shading contribution
				closest = shade(r.evalPoint(close), scene[closestIndex]->getNormal(glm::vec3(0, 0, 0)), scene[closestIndex]->diffuseColor, close, ofColor::lightGray, power, r);
				image.setColor(i, j, closest);
			}
			else if (background) {
				image.setColor(i, j, ofColor::black);
			}
		}
	}

	image.save("output.png");
	image.load("output.png");

	cout << "render saved" << endl;
}

// --------------------------------------------------------------
//calculates ambient shading
//returns shaded color
ofColor ofApp::ambient(const ofColor diffuse) {
	ofColor ambient = ofColor(0, 0, 0);
	ambient = .05 * diffuse;
	return ambient;
}


//--------------------------------------------------------------
//calculates lambert shading
//returns shaded color
ofColor ofApp::lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, Ray r, Light light) {
	ofColor lambert = ofColor(0, 0, 0);
	float zero = 0.0;
	//
	//distance = light.pos - p
	//
	glm::vec3 l = glm::normalize(light.position - p);
	lambert += diffuse * (light.intensity / distance * distance) * (glm::max(zero, glm::dot(norm, l)));

	return lambert;
}


//--------------------------------------------------------------
//calculates all shading including:
// lambert
// phong
// ambient
//returns shaded color
ofColor ofApp::phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, const ofColor specular, float power, float distance, Ray r, Light light) {
	ofColor phong = ofColor(0, 0, 0);
	glm::vec3 h = glm::vec3(0);

	glm::vec3 l = glm::normalize(light.position - p);
	glm::vec3 v = glm::normalize(renderCam.position - p);
	h = glm::normalize(l + v);

	float distance1 = glm::distance(light.position, p);


	//phong += (ambient(diffuse)) + (lambert(p, norm, diffuse, distance1, r, light)) + (specular * (light.intensity / distance1 * distance1) * glm::pow(glm::max(zero, glm::dot(norm, h)), power));
	phong += (lambert(p, norm, diffuse, distance1, r, light)) + (specular * (light.intensity / distance1 * distance1) * glm::pow(glm::max(zero, glm::dot(norm, h)), power));


	return phong;
}

//--------------------------------------------------------------
//calculates lambert shading from spot lights
//returns shaded color
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

ofColor ofApp::spotLightLambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, Ray r, spotLight light) {
	ofColor lambert = ofColor(0, 0, 0);
	float zero = 0.0;
	float inf = FLT_MAX;
	glm::vec3 point, normal;
	//
	//distance = light.pos - p
	//
	//if p is inside cone illumination area


	Ray s = Ray(renderCam.position, glm::normalize(p-renderCam.position ));
	if (glm::intersectRaySphere(s.p, s.d, light.aimPoint, light.coneHeight/2, point, normal)) {
		glm::vec3 l = glm::normalize(light.position - p);
		lambert += diffuse * (light.intensity / distance * distance) * (glm::max(zero, glm::dot(norm, l)));
	}


	//if l is inside the cone then lambert
	//l *= light.coneAngle;

	return lambert;
}


//--------------------------------------------------------------
//adds shading contribution
//calculates shadows
//returns shaded color
ofColor ofApp::shade(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, float distance, const ofColor specular, float power, Ray r) {
	ofColor shaded = (0, 0, 0);
	glm::vec3 p1 = p;

	//loop through all lights
	for (int i = 0; i < light.size(); i++) {
		blocked = false;

		//test for shadows
		if (closestIndex < 2) {								//if the closest object is one of the planes

			//ground plane shadows
			//
			if (scene[0]->intersect(r, p1, glm::vec3(0, 1, 0))) {													//check if current point intersected with ground plane

				Ray shadowRay = Ray(scene[0]->getIntersectionPoint(), light[i]->position - scene[0]->getIntersectionPoint());

				//check all sphere objects
				for (int j = 2; j < scene.size(); j++) {
					if (scene[j]->intersect(shadowRay, p1, scene[j]->getNormal(p1))) {
						blocked = true;
					}
				}
			}

			//wall plane shadows
			//
			if (scene[1]->intersect(r, p1, glm::vec3(0, 0, 1))) {													//check if current point intersected with ground plane

				Ray shadowRay = Ray(scene[1]->getIntersectionPoint(), light[i]->position - scene[1]->getIntersectionPoint());

				//check all sphere objects
				for (int j = 2; j < scene.size(); j++) {
					if (scene[j]->intersect(shadowRay, p1, scene[j]->getNormal(p1))) {
						blocked = true;
					}
				}
			}
		}
		if (!blocked) {
			//add shading contribution for current light
			shaded += phong(p, norm, diffuse, specular, power, distance, r, *light[i]);
		}
	}

	//pointLights shading

	for (int i = 0; i < spotLights.size(); i++) {
		shaded += spotLightLambert(p, norm, diffuse, distance, r, *spotLights[i]);
		//shaded += phong(p, norm, diffuse, specular, power, distance, r, *pointLights[i]);

	}



	return shaded;
}
//--------------------------------------------------------------
void ofApp::draw() {

	ofSetDepthTest(true);

	theCam->begin();

	scene.clear();

	scene.push_back(new Plane(glm::vec3(0, -5, 0), glm::vec3(0, 1, 0), ofColor::darkBlue, 600, 400));				//ground plane


	scene.push_back(new Sphere(glm::vec3(0, 1, -2), 1, ofColor::purple));											//purple sphere

	//scene.push_back(new Sphere(glm::vec3(-1, 0, 1), 1, ofColor::blue));												//blue sphere

	//scene.push_back(new Sphere(glm::vec3(.5, 0, 0), 1, ofColor::darkGreen));											//green sphere


	light.clear();

	light.push_back(new Light(glm::vec3(100, 150, 150), .2));			//top right light

	//light.push_back(new Light(glm::vec3(-20, 30, 45), .2));		//top left light

	//light.push_back(new Light(glm::vec3(-5, -2, 20), .2));				//bottom light

	spotLights.clear();

	//pointLights.push_back(new pointLight(glm::vec3(-20, 30, 45), glm::vec3(1,1,1), .2, angle));

	for (int i = 0; i < aimPoint.size(); i++) {
		spotLights.push_back(new spotLight(spotLightPos[i], aimPoint[i], 2, angle[i]));
	}



	for (int i = 0; i < scene.size(); i++) {
		ofColor color = scene[i]->diffuseColor;
		ofSetColor(color);
		scene[i]->draw();
	}


	//draw all lights
	for (int i = 0; i < light.size(); i++) {
		light[i]->setIntensity(intensity);
		light[i]->draw();
	}

	//draw all spotlights
	for (int i = 0; i < spotLights.size(); i++) {
		spotLights[i]->setIntensity(intensity);
		spotLights[i]->draw();
	}


	Ray s = Ray(renderCam.position, glm::normalize(renderCam.position - spotLights[0]->position));
	s.draw(50);


	theCam->end();


	if (!bHide) {
		ofSetDepthTest(false);
		gui.draw();
	}

	//draw render
	if (drawImage) {
		ofSetColor(ofColor::white);
		image.draw(0, 0);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &previewCam;
		break;
	case OF_KEY_F4:
		theCam = &sceneCam;
		break;
	case 'd':
		drawImage = !drawImage;
		break;
	case 't':
		rayTrace();
		break;
	case 'h':
		bHide = !bHide;
		break;
	case OF_KEY_UP:
		updateAngle(true);
		break;
	case OF_KEY_DOWN:
		updateAngle(false);
		break;
	case OF_KEY_RIGHT:
		angle[0] += .5;
		break;
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	if (!mainCam.getMouseInputEnabled()) {

		if (aimPointDrag) {
			planeNormal = glm::normalize(mainCam.getPosition() - aimPoint[0]);
			p = Plane(aimPoint[0], planeNormal);

			glm::vec3 screen3dpt = theCam->screenToWorld(glm::vec3(x, y, 0));
			glm::vec3 rayOrigin = theCam->getPosition();
			glm::vec3 rayDir = glm::normalize(screen3dpt - rayOrigin);
			r = Ray(rayOrigin, rayDir);
			p.intersect(r, glm::vec3(0), glm::vec3(0));

			
			aimPoint[lightIndex] = p.getIntersectionPoint();

		}

		if (lightDrag) {
			planeNormal = glm::normalize(mainCam.getPosition() - aimPoint[0]);
			p = Plane(aimPoint[0], planeNormal);

			glm::vec3 screen3dpt = theCam->screenToWorld(glm::vec3(x, y, 0));
			glm::vec3 rayOrigin = theCam->getPosition();
			glm::vec3 rayDir = glm::normalize(screen3dpt - rayOrigin);
			r = Ray(rayOrigin, rayDir);
			p.intersect(r, glm::vec3(0), glm::vec3(0));


			spotLightPos[lightIndex] = p.getIntersectionPoint();
		}
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	glm::vec3 screen3dpt = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 rayOrigin = theCam->getPosition();
	glm::vec3 rayDir = glm::normalize(screen3dpt - rayOrigin);
	bool aimPointSelect = false;
	bool lightSelect = false;


	r = Ray(rayOrigin, rayDir);
	glm::vec3 point, normal;
	for (int i = 0; i < spotLights.size(); i++) {
		aimPointSelect = glm::intersectRaySphere(r.p, r.d, spotLights[i]->aimPoint, spotLights[i]->coneHeight, point, normal);
		if (aimPointSelect) {
			lightIndex = i;
			aimPointDrag = true;


		}


		lightSelect = glm::intersectRaySphere(r.p, r.d, spotLights[i]->position, spotLights[i]->coneHeight, point, normal);
		if (lightSelect) {
			lightIndex = i;
			lightDrag = true;
			spotLights[i]->lightSelected = true;



		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	lightDrag = false;
	aimPointDrag = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
