#ifndef OFX_SVGTYPES
#define OFX_SVGTYPES

#include "ofxVec2f.h"
#include "ofxDisplayList.h"

enum {
    ofxSVGObject_Text,
    ofxSVGObject_Rectangle,
    ofxSVGObject_Circle,
    ofxSVGObject_Ellipse,
    ofxSVGObject_Line,
    ofxSVGObject_Polygon,
	ofxSVGObject_Polyline,
    ofxSVGObject_Path,
	
    ofxSVGRender_DirectMode,
    ofxSVGRender_DisplayList,
    ofxSVGRender_VertexArray,
    ofxSVGRender_VertexBufferObject,
	
	ofxSVGVector_Point,
	ofxSVGVector_BezierPoint,
};

// GENERIC OBJECT
//-------------------------------------------------

class ofxSVGObject {
public:
    void draw(){
        switch(renderMode){
            case ofxSVGRender_DirectMode:
				break;
            case ofxSVGRender_DisplayList:
                dl.draw();
				break;
            case ofxSVGRender_VertexArray:
				break;
            case ofxSVGRender_VertexBufferObject:
				break;
            default:
                printf("OfxSVGObject: RenderMode Undefined");
				break;
        }
    }
    int             renderMode;
    int             type;
	
    string          name;
	
    int             fill;
    int             stroke;
    int             strokeWeight;
    float           opacity;
	
    ofxDisplayList  dl;
	
    vector<ofPoint> vertexs;
};

class ofxSVGRectangle : public ofxSVGObject {
public:
    float x, y, width, height;
};

class ofxSVGEllipse : public ofxSVGObject {
public:
    float x, y, rx, ry;
};

class ofxSVGCircle : public ofxSVGObject {
public:
    float x, y, r;
};

class ofxSVGLine : public ofxSVGObject {
public:
    float x1, y1, x2, y2;
};

class ofxSVGText : public ofxSVGObject {
public:
    vector<ofPoint> positions;
    vector<string>  texts;
    vector<string>  fonts;
    vector<int>  colors;
	//ofTTFCharacters	characters; // these can be enabled later
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
	int type;
	ofPoint p, c1, c2;
};
class ofxSVGPath : public ofxSVGObject {
public:
	vector<ofxSVGPoint> vectorData;
};

class ofxComplexSVGPath : public ofxSVGObject {
	public:
		vector<vector<ofxVec2f>*> paths;
};

class ofxSVGImage : public ofxSVGObject {
public:
	ofTexture *tex;
};


// LAYER
//-------------------------------------------------

class ofxSVGLayer {
public:
	
    void draw(){
        for(int i=0; i<objects.size(); i++){
            objects[i]->draw();
        }
    }

    string                  name;
    vector<ofxSVGObject*>    objects;
};


#endif
