#pragma once



enum {
    ofxSVGObject_Text,
    ofxSVGObject_Rectangle,
    ofxSVGObject_Circle,
    ofxSVGObject_Ellipse,
    ofxSVGObject_Line,
    ofxSVGObject_Polygon,
	ofxSVGObject_Polyline,
    ofxSVGObject_Path,
	
    ofxSVGRender_Texture,
    ofxSVGRender_VertexArray,
    ofxSVGRender_VBO,
	ofxSVGRender_FBO,
	
	ofxSVGVector_Point,
	ofxSVGVector_BezierPoint,
};

// GENERIC OBJECT
//-------------------------------------------------

class ofxSVGObject {
public:
    
	void draw() {}
	
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
};

class ofxSVGRectangle : public ofxSVGObject {
public:
    float x, y, width, height;
	
	void draw() {
		ofSetLineWidth(strokeWeight);
		ofFill();
		ofSetColor(fillColor);
		ofRect(x, y, width, height);
		ofSetColor(strokeColor);
		ofRect(x, y, width, height);
	}
};

class ofxSVGEllipse : public ofxSVGObject {
public:
    float x, y, rx, ry;
	
	void draw() {
		ofSetLineWidth(strokeWeight);
		ofFill();
		ofSetColor(fillColor);
		ofEllipse(x, y, rx, ry);
		ofNoFill();
		ofSetColor(strokeColor);
		ofRect(x, y, rx, ry);
	}
};

class ofxSVGCircle : public ofxSVGObject {
public:
    float x, y, r;
	
	void draw() {
		ofSetLineWidth(strokeWeight);
		ofFill();
		ofSetColor(fillColor);
		ofCircle(x, y, r);
		ofNoFill();
		ofSetColor(strokeColor);
		ofCircle(x, y, r);
	}
};

class ofxSVGLine : public ofxSVGObject {
public:
    float x1, y1, x2, y2;
	
	void draw() {
		ofSetLineWidth(strokeWeight);
		ofSetColor(strokeColor);
		ofLine(x1, y1, x2, y2);
	}
};

class ofxSVGText : public ofxSVGObject {
public:
    vector<ofPoint> positions;
    vector<string>  texts;
    vector<string>  fonts;
    vector<int>  colors;
	//ofTTFCharacters	characters; // these can be enabled later
	
	void draw() {
		/*ofSetLineWidth(strokeWeight);
		ofFill();
		ofSetColor(fillColor);
		ofCircle(x, y, r);
		ofNoFill();
		ofSetColor(strokeColor);
		ofCircle(x, y, r);*/
	}
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
	//vector<ofxSVGPoint> vectorData;
	ofPath path;
	
	void draw() {
		ofSetLineWidth(strokeWeight);
		ofFill();
		ofSetColor(fillColor);
		path.draw();
	}
	
};

class ofxComplexSVGPath : public ofxSVGObject {
	public:
	
	vector< vector<ofVec2f> > paths;
	void newVector() {
		vector<ofVec2f> v;
		paths.push_back(v);
	}
	
	void addPoint(ofVec2f pt) {
		paths.back().push_back(pt);
	}
	
	ofVec2f getLastPoint(ofVec2f pt) {
		return paths.back().back();
	}
	
	vector<ofVec2f> &operator [] (unsigned int i) {
		return paths[i];
	}
	
};

class ofxSVGImage : public ofxSVGObject {
public:
	ofTexture *tex;
};


// LAYER
//-------------------------------------------------

class ofxSVGLayer {
public:
	
	ofxSVGLayer( int mode = 0 ) {
		// if the layers need to have their own objects, then we need to declare
		// them with those
		switch ( renderMode ) {
			case ofxSVGRender_VBO:
				layerVBO = new ofVbo();
				break;
			case ofxSVGRender_FBO:
				layerFBO = new ofFbo();
				break;
			case ofxSVGRender_Texture:
				layerTex = new ofTexture();
				break;
			default:
				break;
		}
	}
	
	void render() {
		
		switch ( renderMode ) {
			case ofxSVGRender_VBO:
				
				layerVBO->bind();
				for(int i=0; i<objects.size(); i++){
					objects[i]->draw();
				}
				
				layerVBO->unbind();
				break;
			case ofxSVGRender_FBO:
				layerFBO->bind();
				for(int i=0; i<objects.size(); i++){
					objects[i]->draw();
				}
				
				layerFBO->unbind();
				break;
			case ofxSVGRender_Texture:
				layerTex->bind();
				for(int i=0; i<objects.size(); i++){
					objects[i]->draw();
				}
				
				layerTex->unbind();
				break;
			default:
			{
				for(int i=0; i<objects.size(); i++){
					objects[i]->draw();
				}
			}
				break;
		}
		
	}
	
    void draw()
	{
		
		switch ( renderMode ) {
			case ofxSVGRender_VBO:
				//layerVBO->draw(GL_QUADS); // this isn't ready yet
				break;
			case ofxSVGRender_FBO:
				layerFBO->draw(0, 0);
				break;
			case ofxSVGRender_Texture:
				layerTex->draw(0, 0);
				break;
			default:
			{
				for(int i=0; i<objects.size(); i++){
					objects[i]->draw();
					}
				}
				break;
		}
    }

    string                  name;
    vector<ofxSVGObject*>   objects;
	int						renderMode;
	
	ofVbo *layerVBO;
	ofFbo *layerFBO;
	ofTexture *layerTex;
};


