#pragma once

#include "ofMain.h"

enum {
    ofxSVGObject_Text,
    ofxSVGObject_Rectangle,
    ofxSVGObject_Circle,
    ofxSVGObject_Ellipse,
    ofxSVGObject_Line,
    ofxSVGObject_Polygon,
	ofxSVGObject_Polyline,
    ofxSVGObject_Path,
	
	ofxSVGVector_Point,
	ofxSVGVector_BezierPoint,
};

enum SVGDrawingMode { DRAW_VERTEX_ARRAY, DRAW_FBO, DRAW_TEXTURE, DRAW_VBO };

// GENERIC OBJECT
//-------------------------------------------------

class ofxSVGObject {
public:
    
	virtual void draw() {}
	
    int             renderMode;
    int             type;
	
    string          name;
	
    int             fill;
    int             stroke;
    int             strokeWeight;
	ofColor			fillColor;
	ofColor			strokeColor;
    float           opacity;
	
    vector<ofPoint> vertexs;
	
	ofxSVGObject() {
		fill = 0;
		strokeWeight = 0;
		stroke = 0;
	};
	
	virtual void render() {}
};

class ofxSVGRectangle : public ofxSVGObject {
public:
    float x, y, width, height;
	
	void draw() {
		if(strokeWeight != 0)
			ofSetLineWidth(strokeWeight);
		else 
			ofSetLineWidth(1);

			
		if(fill) {
			ofFill();
			ofSetColor(fillColor);
			ofRect(x, y, width, height);
			ofNoFill();
		}
		ofSetColor(strokeColor);
		ofRect(x, y, width, height);
	}
	
	void render() {}
};

class ofxSVGEllipse : public ofxSVGObject {
public:
    float x, y, rx, ry;
	
	void draw() {
		if(strokeWeight != 0)
			ofSetLineWidth(strokeWeight);
		else 
			ofSetLineWidth(1);
		
		if(fill) {
			ofFill();
			ofSetColor(fillColor);
			ofEllipse(x, y, rx, ry);
			ofNoFill();
		}
		ofSetColor(strokeColor);
		ofEllipse(x, y, rx, ry);
		
	}
	
	void render() {}
};

class ofxSVGCircle : public ofxSVGObject {
public:
    float x, y, r;
	
	void draw() {
		
		if(strokeWeight != 0)
			ofSetLineWidth(strokeWeight);
		else 
			ofSetLineWidth(1);
		
		
		if(fill) {
			ofFill();
			ofSetColor(fillColor);
			ofCircle(x, y, r);
			ofNoFill();	
		}
		
		ofSetColor(strokeColor);
		ofCircle(x, y, r);
	}
	
	void render() {}
};

class ofxSVGLine : public ofxSVGObject {
public:
    float x1, y1, x2, y2;
	
	void draw() {
		if(strokeWeight != 0)
			ofSetLineWidth(strokeWeight);
		else 
			ofSetLineWidth(1);
		
		ofSetColor(strokeColor);
		ofLine(x1, y1, x2, y2);
	}
	
	void render() {}
};

class ofxSVGText : public ofxSVGObject {
	
public:
	
    ofVec2f position;
    string text;
    string fontName;
	int fontSize;
    ofColor color;
	ofTrueTypeFont font;
	
	void draw() {
		ofSetColor(color);
		if(!font.isLoaded()) {
			font.loadFont(fontName, fontSize);
		}
		font.drawString(text, position.x, position.y);
		
	}
	
	void render() {}
};

class ofxSVGPolygon : public ofxSVGObject {
public:
};

class ofxSVGPoint {
public:
	ofxSVGPoint(float x, float y){
		p = ofPoint(x,y);
		type = ofxSVGVector_Point;
	}
	
	ofxSVGPoint(float c1x, float c1y, float c2x, float c2y, float px2, float py2){
		p = ofPoint(px2,py2);
		c1 = ofPoint(c1x,c1y);
		c2 = ofPoint(c2x,c2y);
		type = ofxSVGVector_BezierPoint;
	}
	
	void render() {}
	
	int type;
	ofPoint p, c1, c2;
};
class ofxSVGPath : public ofxSVGObject {
public:

	ofPath path;
	
	void draw() {
		if(strokeWeight != 0)
			ofSetLineWidth(strokeWeight);
		else 
			ofSetLineWidth(1);
		
		if(fill) {
			ofFill();
			ofSetColor(fillColor);
		} else {
			ofNoFill();
		}
		ofSetColor(strokeColor);
		path.draw();
	}
	
	void render() {
		path.setStrokeColor(strokeColor);
		path.setFilled(fill);
		path.setFillColor(fillColor);
	}
	
};

class ofxComplexSVGPath : public ofxSVGObject {
	public:
	
	ofPath path;
	
	void draw() {
		if(strokeWeight != 0)
			ofSetLineWidth(strokeWeight);
		else 
			ofSetLineWidth(1);
		
		if(fill) {
			ofFill();
			ofSetColor(fillColor);
		} else {
			ofNoFill();
		}
		ofSetColor(strokeColor);
		
		path.draw();
	}
	
	void render() {
	
		path.setStrokeColor(strokeColor);
		path.setFilled(fill);
		path.setFillColor(fillColor);
		path.simplify(0.5);
		
	}
	
};

class ofxSVGImage : public ofxSVGObject {
public:
	ofTexture *tex;
	
	void render() {}
};


// LAYER
//-------------------------------------------------

class ofxSVGLayer {
public:
	
	ofxSVGLayer( SVGDrawingMode mode = DRAW_VERTEX_ARRAY) {
		// if the layers need to have their own objects, then we need to declare
		// them with those
		
		renderMode = mode;
		
		switch ( renderMode ) {
			case DRAW_VBO:
				layerVBO = new ofVbo();
				break;
			case DRAW_FBO:
				layerFBO = new ofFbo();
				break;
			case DRAW_TEXTURE:
				layerTex = new ofTexture();
				break;
			case DRAW_VERTEX_ARRAY:
				break;
		}
	}
	
	void render() {
		
		switch ( renderMode ) {
			case DRAW_VBO:
				
				layerVBO->bind();
				for(int i=0; i<objects.size(); i++){
					objects[i]->draw();
				}
				
				layerVBO->unbind();
				break;
			case DRAW_FBO:
				layerFBO->bind();
				for(int i=0; i<objects.size(); i++){
					objects[i]->draw();
				}
				
				layerFBO->unbind();
				break;
			case DRAW_TEXTURE:
				layerTex->bind();
				for(int i=0; i<objects.size(); i++){
					objects[i]->render();
					objects[i]->draw();
				}
				
				layerTex->unbind();
				break;
			case DRAW_VERTEX_ARRAY:
				for(int i=0; i<objects.size(); i++){
					objects[i]->render();
				}
			break;
		}
		
	}
	
    void draw()
	{
		
		switch ( renderMode ) {
			case DRAW_VBO:
				//layerVBO->draw(GL_QUADS); // this isn't ready yet
				break;
			case DRAW_FBO:
				layerFBO->draw(0, 0);
				break;
			case DRAW_TEXTURE:
				layerTex->draw(0, 0);
				break;
			case DRAW_VERTEX_ARRAY:
				for(int i=0; i<objects.size(); i++){
					objects[i]->draw();
				}
			break;
		}
    }

    string                  name;
    vector<ofxSVGObject*>   objects;
	SVGDrawingMode			renderMode;
	
	ofVbo *layerVBO;
	ofFbo *layerFBO;
	ofTexture *layerTex;
	ofMatrix4x4 mat;
};


