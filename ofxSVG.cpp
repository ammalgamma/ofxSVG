#include "ofxSVG.h"

//--------------------------------------------------------------
void ofxSVG::load(string svgPath){

    bVerbose = true;

    if(bVerbose){
        cout<<"ofxSVG: Loading \""<<svgPath<<"\""<<endl<<endl;
    }

    TiXmlBase::SetCondenseWhiteSpace(false);

    ifstream file(ofToDataPath(svgPath).c_str());

    // Check if File Exist
    //--------------------------------------------
    if (!file && bVerbose){
        printf("ofxSVG: File not found.");
        return;
    }
    else {
        // Extract string from file
        //----------------------------------------
        string line;
        string svgString;
        while (getline(file, line)) {
            svgString += "\n"+line;
        }

        // Check if SVG
        //----------------------------------------
        if(svgString.find("<svg") == -1 && bVerbose){
            printf("ofxSVG: Unknown Format.");
            return;
        }
        else {
            // Remove Header
            //------------------------------------
            int svgTagPos = svgString.find("<svg");
            svgString = svgString.substr(svgTagPos, svgString.length() - svgTagPos);

            // Load File to TinyXml
            //--------------------------------------------
            svgXml.loadFromBuffer(svgString);
        }

        file.close();
    }

    // Get Metadatas
    //--------------------------------------------
    svgVersion  = svgXml.getAttribute("svg", "version", "undefined");

    docWidth    = svgXml.getAttribute("svg", "width", 0);
    docHeight   = svgXml.getAttribute("svg", "height", 0);

    if(bVerbose){
        cout<<"ofxSVG: SVG Version: "<<svgVersion<<endl;
        cout<<"ofxSVG: Document size: "<<docWidth<<"x"<<docHeight<<endl;
    }

    // Enter SVG Main Layer
    //--------------------------------------------
    svgXml.pushTag("svg");

        int pops;
        if(svgXml.tagExists("switch")) {
            svgXml.pushTag("switch");
            pops++;
        }
        if(svgXml.tagExists("g")) {
            svgXml.pushTag("g");
            pops++;
        }

	// while building the display lists, we want smoothing
	// later on it can be disabled
	// -------------------------------
	ofSetCircleResolution(80);
	ofEnableSmoothing();


            // Read Number of Layers
            //----------------------------------------
            int nLayers = svgXml.getNumTags("g");

            // Only one layer
            //----------------------------------------
            if(nLayers == 0){
                if(bVerbose){
                    cout<<"ofxSVG: Loading one layers."<<endl;
                }

                ofxSVGLayer layer;
                layer.name = svgXml.getAttribute("g", "id", "");
                layers.push_back( layer );

                parseLayer();
            }
            // Multiple Layers
            //----------------------------------------
            else {
                if(bVerbose){
                    cout<<"ofxSVG: Loading "<<nLayers<<" layers."<<endl;
                }
                for(int i=0; i<nLayers; i++){
                    ofxSVGLayer layer;
                    layer.name = svgXml.getAttribute("g", "id", "", i);
                    layers.push_back( layer );

                    if(bVerbose){
                        cout<<"ofxSVG:    Layer \""<<layer.name<<"\" :"<<endl;
                    }

                    svgXml.pushTag("g", i);
                        parseLayer();
                    svgXml.popTag();
                }
            }

        for(int i=0;i<pops; i++) svgXml.popTag();

    svgXml.popTag();

    if(bVerbose){
        cout<<endl<<"ofxSVG: Loading succes!"<<endl;
    }

    TiXmlBase::SetCondenseWhiteSpace(true);

	// now that the DLs are built, go ahead and disable blending
	ofDisableSmoothing();
}


ofxSVG::ofxSVG( ofFbo &fbo ) 
{ 
	fboForDrawing = fbo;
	drawingMode = DRAW_FBO;
}

ofxSVG::ofxSVG( ofFbo *fbo ) {
	fboForDrawing = *fbo;
	drawingMode = DRAW_FBO;
}

ofxSVG::ofxSVG( ofTexture &tex ) 
{ 
	texForDrawing = tex;
	drawingMode = DRAW_TEXTURE;
}

ofxSVG::ofxSVG( ofTexture *tex ) {
	texForDrawing = *tex;
	drawingMode = DRAW_TEXTURE;
}

ofxSVG::ofxSVG( ofVbo &vbo ) 
{ 
	vboForDrawing = vbo;
	drawingMode = DRAW_VBO;
}

ofxSVG::ofxSVG( ofVbo *vbo ) {
	vboForDrawing = *vbo;
	drawingMode = DRAW_VBO;
}

ofxSVG::ofxSVG( ) {
	drawingMode = DRAW_VERTEX_ARRAY;
}

ofxSVG::~ofxSVG() {
	
	// lotta cleanup to do here
}

// Parsing
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

void ofxSVG::parseLayer(){
    int layerId = (int) (layers.size()) - 1;

    // Get Number of Objects in this Layer
    //--------------------------------------------
    int numObjects = svgXml.getNumTags();
    if(bVerbose) cout<<"ofxSVG:        numObjects: "<<numObjects<<endl;


    // Loop threw all objects, parse and render them
    //--------------------------------------------
    for(int i=0; i<numObjects; i++) {
        currentIteration = i;

        string name = svgXml.getName(i);

        if(name == "rect")  parseRect();
        else if(name == "circle") parseCircle();
        else if(name == "ellipse") parseEllipse();
        else if(name == "line") parseLine();
        else if(name == "polygon") parsePolygon();
        else if(name == "text") parseText();
        //else if(name == "path")parsePath();
		else if(name == "path")parsePathExperimental(); // not using this yet JN
		else if(name == "image")parseImage();
        else if(name == "g"){
            svgXml.pushTag(i);
                parseLayer();
            svgXml.popTag();
        }
    }

}

GLint ofxSVG::getImageColorType(ofImage &image) {
	GLint imgType;
	int channum = image.getPixelsRef().getNumChannels();
	if(channum == 3) {
		return GL_RGB;
	} else if( channum == 4 ) {
		return GL_RGBA;
	}
	return GL_LUMINANCE;
}

void ofxSVG::parseImage() {
	
	string path = svgXml.getAttribute("xlink:href", currentIteration);
	int imgWidth, imgHeight, x, y;
	imgWidth = atoi(svgXml.getAttribute("width", currentIteration).c_str());
	imgHeight = atoi(svgXml.getAttribute("height", currentIteration).c_str());
	x = atoi(svgXml.getAttribute("x", currentIteration).c_str());
	y = atoi(svgXml.getAttribute("y", currentIteration).c_str());
	ofImage tmpimg;
	tmpimg.loadImage(path);

	ofxSVGImage *img = new ofxSVGImage();
	img->tex = new ofTexture;
	
	img->tex->allocate(tmpimg.width, tmpimg.height, getImageColorType(tmpimg));
	img->tex->loadData(tmpimg.getPixels(), tmpimg.width, tmpimg.height, getImageColorType(tmpimg));
	
	img->tex->draw(x, y, imgWidth, imgHeight);
	
	layers[layers.size()-1].objects.push_back(img);
}

//-------------------------------------------------------------------------------------

void ofxSVG::parseRect(){
    string xStr     = svgXml.getAttribute("x", currentIteration);
    string yStr     = svgXml.getAttribute("y", currentIteration);
    float x         = (xStr!="") ? ofToFloat(xStr) : 0.0f;
    float y         = (yStr!="") ? ofToFloat(yStr) : 0.0f;
    float width     = ofToFloat(svgXml.getAttribute("width", currentIteration));
    float height    = ofToFloat(svgXml.getAttribute("height", currentIteration));
    string id       = svgXml.getAttribute("id", currentIteration);

    // Extract Rotation from
    //------------------------------------
    string transform = svgXml.getAttribute("transform", currentIteration);
	string opacity = svgXml.getAttribute("opacity", currentIteration);
	string fill = svgXml.getAttribute("fill", currentIteration);
    string stroke = svgXml.getAttribute("stroke", currentIteration);
    float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;

    if(!(fill=="none" && stroke=="")){

        ofxSVGRectangle* obj = new ofxSVGRectangle;

        // Shape info
        //--------------------------------
        obj->name        = id;
        obj->x           = x;
        obj->y           = y;
        obj->width       = width;
        obj->height      = height;

        // Display List
        //--------------------------------
		
        ofPushMatrix();

        if(transform!=""){
            float rot = rotFromMatrix(transform);
            glTranslatef(-width/2,-height/2,0);
            glRotatef(rot, 0, 0, 1);
        }

        if(fill!="none"){
            ofFill();
			if(opacity!="")
				ofEnableAlphaBlending();
            if(fill!=""){
				int rgb = strtol(("0x"+fill.substr(1, fill.length()-1)).c_str(), NULL, 0);
				float r = (rgb >> 16) & 0xFF;
				float g = (rgb >> 8) & 0xFF;
				float b = (rgb) & 0xFF;
				ofSetColor(r,g,b,alpha);
			}
            else ofSetColor(0,0,0,alpha);
            ofRect(x, y,width,height);
			ofDisableAlphaBlending();
        }

        if(stroke!="" && stroke!="none"){
            string strokeWeight = svgXml.getAttribute("stroke-width", currentIteration);
            if(strokeWeight!="") ofSetLineWidth(ofToInt(strokeWeight));
            ofNoFill();
            int rgb = strtol(("0x"+stroke.substr(1, stroke.length()-1)).c_str(), NULL, 0);
            float r = (rgb >> 16) & 0xFF;
            float g = (rgb >> 8) & 0xFF;
            float b = (rgb) & 0xFF;
            ofSetColor(r,g,b,alpha);
            ofRect(x,y,width,height);
            if(strokeWeight!="") ofSetLineWidth(1);
        }

        glPopMatrix();

        // Vertexs
        //--------------------------------
        obj->vertexs.push_back(ofPoint(x, y));
        obj->vertexs.push_back(ofPoint(x+width, y));
        obj->vertexs.push_back(ofPoint(x+width, y+height));
        obj->vertexs.push_back(ofPoint(x, y+height));

        layers[layers.size()-1].objects.push_back(obj);
    }
}

//-------------------------------------------------------------------------------------

void ofxSVG::parseCircle(){
    string xStr     = svgXml.getAttribute("cx", currentIteration);
    string yStr     = svgXml.getAttribute("cy", currentIteration);
    float x         = (xStr!="") ? ofToFloat(xStr) : 0.0f;
    float y         = (yStr!="") ? ofToFloat(yStr) : 0.0f;
    float r     = ofToFloat(svgXml.getAttribute("r", currentIteration));

    string id       = svgXml.getAttribute("id", currentIteration);
    string fill = svgXml.getAttribute("fill", currentIteration);
    string stroke = svgXml.getAttribute("stroke", currentIteration);
    string opacity = svgXml.getAttribute("opacity", currentIteration);
    float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;



    ofxSVGCircle* obj = new ofxSVGCircle;

    // Shape info
    //--------------------------------

    obj->type        = ofxSVGObject_Circle;
    obj->name        = id;
    obj->x           = x;
    obj->x           = y;
    obj->r           = r;

    // Display List
    //--------------------------------
    beginRenderer();

    if(fill!="none"){
        ofFill();
        if(opacity!="")
            ofEnableAlphaBlending();
        if(fill!=""){
            int rgb = strtol(("0x"+fill.substr(1, fill.length()-1)).c_str(), NULL, 0);
            float r = (rgb >> 16) & 0xFF;
            float g = (rgb >> 8) & 0xFF;
            float b = (rgb) & 0xFF;
            ofSetColor(r,g,b,alpha);
        }
        else ofSetColor(0,0,0,alpha);
        ofCircle(x,y,r);
		ofDisableAlphaBlending();
    }

    if(stroke!="" && stroke!="none"){
        string strokeWeight = svgXml.getAttribute("stroke-width", currentIteration);
        if(strokeWeight!="") ofSetLineWidth(ofToInt(strokeWeight));
        ofNoFill();
		if(opacity!="") {
			ofEnableAlphaBlending();
		}
        int rgb = strtol(("0x"+stroke.substr(1, stroke.length()-1)).c_str(), NULL, 0);
        if(rgb!=0){
            float r = (rgb >> 16) & 0xFF;
            float g = (rgb >> 8) & 0xFF;
            float b = (rgb) & 0xFF;
            ofSetColor(r,g,b,alpha);
        }
        else ofSetColor(0,0,0,alpha);
        ofSetLineWidth(1);
        ofCircle(x,y,r);
        if(strokeWeight!="") ofSetLineWidth(1);
		ofDisableAlphaBlending();
    }

    endRenderer();

    // Vertexs
    //--------------------------------

    int res = 30;
    float angle = 0.0f;
    float theta = M_TWO_PI / (float) res;
	for(int i = 0; i < res; i++){
		obj->vertexs.push_back(ofPoint(x + cos(angle) * r, y + sin(angle) * r));
		angle+=theta;
	}

    layers[layers.size()-1].objects.push_back(obj);
}

//-------------------------------------------------------------------------------------

void ofxSVG::parseEllipse(){
    string xStr     = svgXml.getAttribute("cx", currentIteration);
    string yStr     = svgXml.getAttribute("cy", currentIteration);
    float x         = (xStr!="") ? ofToFloat(xStr) : 0.0f;
    float y         = (yStr!="") ? ofToFloat(yStr) : 0.0f;
    float rx     = ofToFloat(svgXml.getAttribute("rx", currentIteration))*2.0;
    float ry     = ofToFloat(svgXml.getAttribute("ry", currentIteration))*2.0;

    string id       = svgXml.getAttribute("id", currentIteration);
    string fill = svgXml.getAttribute("fill", currentIteration);
    string stroke = svgXml.getAttribute("stroke", currentIteration);
    string opacity = svgXml.getAttribute("opacity", currentIteration);
    float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;


    ofxSVGEllipse* obj = new ofxSVGEllipse;

    // Shape info
    //--------------------------------

    obj->type        = ofxSVGObject_Ellipse;

    obj->name        = id;
    obj->x           = x;
    obj->y           = x;
    obj->rx          = rx;
    obj->ry          = ry;

    // Display List
    //--------------------------------
    beginRenderer();


    if(fill!="" && fill!="none"){
        ofFill();
        if(opacity!="")
            ofEnableAlphaBlending();
        if(fill!=""){
            int rgb = strtol(("0x"+fill.substr(1, fill.length()-1)).c_str(), NULL, 0);
            float r = (rgb >> 16) & 0xFF;
            float g = (rgb >> 8) & 0xFF;
            float b = (rgb) & 0xFF;
            ofSetColor(r,g,b,alpha);
        }
        else ofSetColor(0,0,0,alpha);
        ofEllipse(x,y,rx,ry);
		ofDisableAlphaBlending();
    }

    if(stroke!="" && stroke!="none"){
        string strokeWeight = svgXml.getAttribute("stroke-width", currentIteration);
        if(strokeWeight!="") ofSetLineWidth(ofToInt(strokeWeight));
        ofNoFill();
		if(opacity!="") {
			ofEnableAlphaBlending();
		}
        int rgb = strtol(("0x"+stroke.substr(1, stroke.length()-1)).c_str(), NULL, 0);
        float r = (rgb >> 16) & 0xFF;
        float g = (rgb >> 8) & 0xFF;
        float b = (rgb) & 0xFF;
        ofSetColor(r,g,b,alpha);
        ofEllipse(x,y,rx,ry);
        if(strokeWeight!="") ofSetLineWidth(1);
		ofDisableAlphaBlending();
    }

    endRenderer();

    // Vertexs
    //--------------------------------

    int res = 300;
    float angle = 0.0f;
    float theta = M_TWO_PI / (float) res;
	for(int i = 0; i < res; i++){
		obj->vertexs.push_back(ofPoint(x + cos(angle) * rx * 0.5f, y + sin(angle) * ry * 0.5f));
		angle+=theta;
	}

    layers[layers.size()-1].objects.push_back(obj);
}

//-------------------------------------------------------------------------------------

void ofxSVG::parseLine(){
    float x1     = ofToFloat(svgXml.getAttribute("x1", currentIteration));
    float y1     = ofToFloat(svgXml.getAttribute("y1", currentIteration));
    float x2     = ofToFloat(svgXml.getAttribute("x2", currentIteration));
    float y2     = ofToFloat(svgXml.getAttribute("y2", currentIteration));

    string id       = svgXml.getAttribute("id", currentIteration);

    string fill = svgXml.getAttribute("fill", currentIteration);
    string stroke = svgXml.getAttribute("stroke", currentIteration);
    string opacity = svgXml.getAttribute("opacity", currentIteration);
    float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;

    ofxSVGLine* obj = new ofxSVGLine;

    // Shape info
    //--------------------------------

    obj->type        = ofxSVGObject_Line;

    obj->name        = id;
    obj->x1          = x1;
    obj->y1          = y1;
    obj->x2          = x2;
    obj->y2          = y2;

    // Display List
    //--------------------------------
    beginRenderer();

    if(stroke!="" && stroke!="none"){
        string strokeWeight = svgXml.getAttribute("stroke-width", currentIteration);
        if(strokeWeight!="") ofSetLineWidth(ofToInt(strokeWeight));
        ofNoFill();
		if(opacity!="") {
			ofEnableAlphaBlending();
		}
        int rgb = strtol(("0x"+stroke.substr(1, stroke.length()-1)).c_str(), NULL, 0);
        float r = (rgb >> 16) & 0xFF;
        float g = (rgb >> 8) & 0xFF;
        float b = (rgb) & 0xFF;
        ofSetColor(r,g,b,alpha);
        ofLine(x1,y1,x2,y2);
        if(strokeWeight!="") ofSetLineWidth(1);
		ofDisableAlphaBlending();
    }

    endRenderer();

    // Vertexs
    //--------------------------------
    obj->vertexs.push_back(ofPoint(x1, y1));
    obj->vertexs.push_back(ofPoint(x2, y2));

    layers[layers.size()-1].objects.push_back(obj);
}

//-------------------------------------------------------------------------------------

void ofxSVG::parsePolygon(){
    string id       = svgXml.getAttribute("id", currentIteration);
    string fill = svgXml.getAttribute("fill", currentIteration);
    string stroke = svgXml.getAttribute("stroke", currentIteration);
    string opacity = svgXml.getAttribute("opacity", currentIteration);
    float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;

    vector<string> pointsStrings = ofSplitString(svgXml.getAttribute("points", currentIteration), " ");

    ofxSVGPolygon* obj = new ofxSVGPolygon;

    // Shape info
    //--------------------------------

    obj->type        = ofxSVGObject_Polygon;
    obj->name        = id;

    // Vertexs
    //--------------------------------
    for(int i=0; i<pointsStrings.size()-1; i++){
        vector<string> pointString = ofSplitString(pointsStrings[i], ",");
        obj->vertexs.push_back(ofVec2f(strtod(pointString[0].c_str(), NULL), strtod(pointString[1].c_str(), NULL)));
    }

	
    beginRenderer();

    if(fill!="none"){
        ofFill();
        if(opacity!="")
            ofEnableAlphaBlending();
        if(fill!=""){
            int rgb = strtol(("0x"+fill.substr(1, fill.length()-1)).c_str(), NULL, 0);
            float r = (rgb >> 16) & 0xFF;
            float g = (rgb >> 8) & 0xFF;
            float b = (rgb) & 0xFF;
            ofSetColor(r,g,b,alpha);
        }
        else ofSetColor(0,0,0,alpha);
        ofBeginShape();
        for(int i=0; i<obj->vertexs.size(); i++) ofVertex(obj->vertexs[i].x, obj->vertexs[i].y);
        ofEndShape(OF_CLOSE);
		ofDisableAlphaBlending();
    }

    if(stroke!="" && stroke!="none"){
        string strokeWeight = svgXml.getAttribute("stroke-width", currentIteration);
        if(strokeWeight!="") ofSetLineWidth(ofToInt(strokeWeight));
        ofNoFill();
		if(opacity!="") {
			ofEnableAlphaBlending();
		}
        int rgb = strtol(("0x"+stroke.substr(1, stroke.length()-1)).c_str(), NULL, 0);
        float r = (rgb >> 16) & 0xFF;
        float g = (rgb >> 8) & 0xFF;
        float b = (rgb) & 0xFF;
        ofSetColor(r,g,b,alpha);

        ofBeginShape();
        for(int i=0; i<obj->vertexs.size(); i++) ofVertex(obj->vertexs[i].x, obj->vertexs[i].y);
        ofEndShape(OF_CLOSE);

        if(strokeWeight!="") ofSetLineWidth(1);
		ofDisableAlphaBlending();
    }

    endRenderer();

    layers[layers.size()-1].objects.push_back(obj);
}

void ofxSVG::parseText(){

    svgXml.pushTag(currentIteration);
    int numTSpans = svgXml.getNumTags("tspan");
    svgXml.popTag();

    // Multiline Text
    //------------------------------------
    if(numTSpans>0){

        // Extract XY From matrix
        //------------------------------------
        ofPoint pos = posFromMatrix(svgXml.getAttribute("transform", currentIteration));

        // Get Alpha
        //------------------------------------
        string opacity = svgXml.getAttribute("opacity", currentIteration);
        float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;

        svgXml.pushTag(currentIteration);

        // Parse and Load Fonts
        //--------------------------------
        for(int j=0; j<numTSpans; j++){
            int fontSize    = svgXml.getAttribute("tspan", "font-size", 0, j);
            string fontName = svgXml.getAttribute("tspan", "font-family", "", j);
            fontName        = fontName.substr(1, fontName.length() - 2);
            string fontExt  = ".ttf";


            // Check if Font is already loaded
            //------------------------------------
            if(fonts.count(fontName+ofToString(fontSize)) == 0){
                    ofTrueTypeFont* font = new ofTrueTypeFont();

                // Find Font Extension
                // and check if file exist
                //--------------------------------
                ifstream ttfFile(ofToDataPath("fonts/"+fontName+fontExt).c_str());
                if(!ttfFile) fontExt = ".otf";

                ifstream otfFile(ofToDataPath("fonts/"+fontName+fontExt).c_str());
                if(!otfFile && bVerbose)
                    printf("ofxSVG: Font file '%s' not found or format not supported\n", fontName.c_str());
                else otfFile.close();

                // Load font and add to font map
                //--------------------------------
					fonts[fontName+ofToString(fontSize)].loadFont("fonts/"+fontName+fontExt, fontSize*0.75f, true, true, true);

                //fonts.insert(make_pair(fontName+ofToString(fontSize), font));
            }
        }

        ofxSVGText* obj = new ofxSVGText;

        // Shape info
        //--------------------------------
        obj->type        = ofxSVGObject_Text;
    
        obj->name        = svgXml.getAttribute("id", currentIteration);

        // Display List
        //--------------------------------
		
        for(int j=0; j<numTSpans; j++){

            // Parse Current Text Metadata
            //------------------------------------
            float x         = svgXml.getAttribute("tspan", "x", 0.0f, j) + pos.x;
            float y         = svgXml.getAttribute("tspan", "y", 0.0f, j) + pos.y;

            string text     = svgXml.getValue("tspan", "", j);

            int fontSize    = svgXml.getAttribute("tspan", "font-size", 0, j);
            string fontName = svgXml.getAttribute("tspan", "font-family", "", j);
            fontName        = fontName.substr(1, fontName.length() - 2);

            obj->positions.push_back(ofPoint(x, y));
            obj->texts.push_back(text);
            obj->fonts.push_back(fontName+ofToString(fontSize));

            // Draw font
            //------------------------------------
            if(svgXml.attributeExists("tspan", "fill", j)){
                string col = svgXml.getAttribute("tspan", "fill", "", j);
                int color = strtol(("0x"+col.substr(1, col.length()-1)).c_str(), NULL, 0);
                float r = (color >> 16) & 0xFF;
                float g = (color >> 8) & 0xFF;
                float b = (color) & 0xFF;
				if(opacity!="") {
					ofEnableAlphaBlending();
				}
                ofSetColor(r,g,b,alpha);
                obj->colors.push_back(color);
            }
            else {
                ofSetColor(0,0,0,alpha);
                obj->colors.push_back(0);
            }

            fonts[fontName+ofToString(fontSize)].drawString(text, x, y);
			ofDisableAlphaBlending();
        }
        endRenderer();

        layers[layers.size()-1].objects.push_back(obj);

        svgXml.popTag();
    }
    // Single Line Text
    //------------------------------------
    else {

        // Extract XY From matrix
        //------------------------------------
        ofPoint pos = posFromMatrix(svgXml.getAttribute("transform", currentIteration));

        // Get Alpha
        //------------------------------------
        string opacity = svgXml.getAttribute("opacity", currentIteration);
        float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;

        // Parse Current Text Metadata
        //------------------------------------
        string text     = svgXml.getValue(currentIteration);

        int fontSize    = strtol(svgXml.getAttribute("font-size", currentIteration).c_str(), NULL, 0);
        string fontName = svgXml.getAttribute("font-family", currentIteration);
        fontName        = fontName.substr(1, fontName.length() - 2);
        string col = svgXml.getAttribute("fill", currentIteration);
        string fontExt  = ".ttf";

        // Check if Font is already loaded
        //------------------------------------
        //if(fonts.count(fontName+ofToString(fontSize)) == 0){

		if(fonts.find(fontName+ofToString(fontSize)) == fonts.end()){
            // Find Font Extension
            // and check if file exist
            //--------------------------------
            ifstream ttfFile(ofToDataPath("fonts/"+fontName+fontExt).c_str());
            if(!ttfFile) fontExt = ".otf";

            ifstream otfFile(ofToDataPath("fonts/"+fontName+fontExt).c_str());
            if(!otfFile && bVerbose)
                printf("ofxSVG: Font file '%s' not found or format not supported\n", fontName.c_str());
            else otfFile.close();

            // Load font and add to font map
            //--------------------------------
                fonts[fontName+ofToString(fontSize)].loadFont("fonts/"+fontName+fontExt, fontSize*0.75f, true, true, true);
        }

        ofxSVGText* obj = new ofxSVGText;

        // Shape info
        //--------------------------------

        obj->type        = ofxSVGObject_Text;
    
        obj->name        = svgXml.getAttribute("id", currentIteration);

        // Display List
        //--------------------------------

        beginRenderer();

        // Draw font
        //------------------------------------
        if(col!="" && col!="none"){
            int color = strtol(("0x"+col.substr(1, col.length()-1)).c_str(), NULL, 0);
            float r = (color >> 16) & 0xFF;
            float g = (color >> 8) & 0xFF;
            float b = (color) & 0xFF;
            ofSetColor(r,g,b,alpha);
        }
        else ofSetColor(0,0,0,alpha);

        fonts[fontName+ofToString(fontSize)].drawString(text, pos.x, pos.y);

        endRenderer();

        layers[layers.size()-1].objects.push_back(obj);
    }

}

//-------------------------------------------------------------------------------------

void ofxSVG::parsePath(){
		string pathStr = svgXml.getAttribute("d", currentIteration);
		
		string sepPath = "";
		
		for(int i = 0; i < pathStr.size(); i++){
			char c = pathStr[i];
			
			if( c < 31 )continue;
			
			if( c == 'M' || c == 'm' ||
			   c == 'L' || c == 'l' ||
			   c == 'V' || c == 'v' ||
			   c == 'H' || c == 'h' ||
			   c == 'C' || c == 'c' ||
			   c == 'S' || c == 's' ||
			   c == 'z' || c == 'Z' )
			{
				sepPath += "*";
				sepPath.push_back(c);
				//sepPath += "*"; 
			}else if(c == '-' || c == ' ' || c == ','){
				if( c == '-'){
					if( sepPath.length() > 0 && pathStr[i-1] >= '0' && pathStr[i-1] <= '9' ){
						sepPath.push_back('|');
					} 
					sepPath.push_back(c);
				}else{
					sepPath.push_back('|');
				}
			}else{
				sepPath.push_back(c);
			}
			
		}
		
		
		string fill = svgXml.getAttribute("fill", currentIteration);
		string stroke = svgXml.getAttribute("stroke", currentIteration);
		string opacity = svgXml.getAttribute("opacity", currentIteration);
		float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;
		
		ofxSVGPath* obj = new ofxSVGPath;
		
		// Shape info
		//--------------------------------
		
		obj->type        = ofxSVGObject_Path;
		obj->name        = svgXml.getAttribute("id", currentIteration);
		
		
		// Path to Vector Data
		//--------------------------------
		//pathToVectorData(sepPath, obj);
		
		// Vector Data to vertexs
		//--------------------------------
		//vectorDataToVertexs(obj, 0.1f);
	
		ofxSVGPathParser parse(&obj->path);
		
		// Display List
		//--------------------------------
		
		beginRenderer();
		
		if(fill!="none"){
			ofFill();
			if(opacity!="")
				ofEnableAlphaBlending();
			if(fill!=""){
				int rgb = strtol(("0x"+fill.substr(1, fill.length()-1)).c_str(), NULL, 0);
				float r = (rgb >> 16) & 0xFF;
				float g = (rgb >> 8) & 0xFF;
				float b = (rgb) & 0xFF;
				ofSetColor(r,g,b,alpha);
			}
			else ofSetColor(0,0,0,alpha);
			
			//drawVectorData(obj);
			obj->path.draw(0, 0);
		}
		
		if(stroke!="" && stroke!="none"){
			string strokeWeight = svgXml.getAttribute("stroke-width", currentIteration);
			if(strokeWeight!="") ofSetLineWidth(ofToInt(strokeWeight));
			ofNoFill();
			int rgb = strtol(("0x"+stroke.substr(1, stroke.length()-1)).c_str(), NULL, 0);
			float r = (rgb >> 16) & 0xFF;
			float g = (rgb >> 8) & 0xFF;
			float b = (rgb) & 0xFF;
			ofSetColor(r,g,b,alpha);
			
			//drawVectorData(obj);
			obj->path.draw(0, 0);
			
			if(strokeWeight!="") ofSetLineWidth(1);
		} else {
			
			ofSetLineWidth(1);
			
		}


		endRenderer();
	
		layers[layers.size()-1].objects.push_back(obj);
}

void ofxSVG::beginRenderer() 
{
	if( drawingMode == DRAW_FBO ) {
		fboForDrawing.begin();
	}
	
	if( drawingMode == DRAW_TEXTURE ) {
		texForDrawing.bind();
	}
}

void ofxSVG::endRenderer()
{
	if( drawingMode == DRAW_FBO ) {
		fboForDrawing.end();
	}
	
	if( drawingMode == DRAW_TEXTURE ) {
		texForDrawing.unbind();
	}
}

bool ofxSVG::isInsidePolygon(ofxSVGPath *path, ofPoint p)
{
	/* Based on code from:
		 http://local.wasp.uwa.edu.au/~pbourke/geometry/insidepoly/
		 adapted to work with openframeworks/ofxSVG by Noto Yota multimedialab, 2010
		 */
	int counter = 0;
	int i,N;
	double xinters;
	
	vector<ofVec2f> points;
	
	vector<ofSubPath>::iterator it = path->path.getSubPaths().begin();
	vector<ofSubPath::Command>::iterator c;
	while (it != path->path.getSubPaths().end()) {
		
		c = it->getCommands().begin();
		
		while( c != it->getCommands().end() ) {
			points.push_back(c->to);
			++c;
		}
		
		++it;
	}


	ofVec2f p1,p2;

	p1 = points[0];
	N = points.size();
	
	for (i=1;i<=N;i++) {
		p2 = points[i % N];
		if (p.y > MIN(p1.y,p2.y)) {
			if (p.y <= MAX(p1.y,p2.y)) {
				if (p.x <= MAX(p1.x,p2.x)) {
					if (p1.y != p2.y) {
						xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
						if (p1.x == p2.x || p.x <= xinters)
							counter++;
						}
					}
				}
			}
		p1 = p2;
		}
	
	if (counter % 2 == 0)
		return(false);
	else
	return(true);
}


/*
 * JN - This is still experimental and not quite ready for primetime, however, feel free to play with it
 */

void ofxSVG::parsePathExperimental() {
	string pathStr = svgXml.getAttribute("d", currentIteration);
	
	ofxSVGPath* obj = new ofxSVGPath();
	obj->type        = ofxSVGObject_Path;
	obj->name        = svgXml.getAttribute("id", currentIteration);
	
	ofxSVGPathParser parser(&obj->path);
	
	const char* c = pathStr.c_str();
	
	parser.parse(&c);
	
	string fill = svgXml.getAttribute("fill", currentIteration);
	string stroke = svgXml.getAttribute("stroke", currentIteration);
	string opacity = svgXml.getAttribute("opacity", currentIteration);
	float alpha = (opacity=="") ? 255.0f : ofToFloat(opacity) * 255.0f;
	
	// Shape info
	//--------------------------------
	
	//obj->type        = ofxSVGPath;
	//obj->name        = svgXml.getAttribute("id", currentIteration);
	
	
	// Display List
	//--------------------------------
	
	beginRenderer();
	
	if(fill!="none"){
		ofFill();
		if(opacity!="")
			ofEnableAlphaBlending();
		if(fill!=""){
			int rgb = strtol(("0x"+fill.substr(1, fill.length()-1)).c_str(), NULL, 0);
			float r = (rgb >> 16) & 0xFF;
			float g = (rgb >> 8) & 0xFF;
			float b = (rgb) & 0xFF;
			ofSetColor(r,g,b,alpha);
		}
		else ofSetColor(0,0,0,alpha);
		
		// this can just be path.draw();
		obj->path.draw(0, 0); //drawVectorDataExperimental(obj);
	}
	
	if(stroke!="" && stroke!="none"){
		string strokeWeight = svgXml.getAttribute("stroke-width", currentIteration);
		if(strokeWeight!="") ofSetLineWidth(ofToInt(strokeWeight));
		ofNoFill();
		int rgb = strtol(("0x"+stroke.substr(1, stroke.length()-1)).c_str(), NULL, 0);
		float r = (rgb >> 16) & 0xFF;
		float g = (rgb >> 8) & 0xFF;
		float b = (rgb) & 0xFF;
		ofSetColor(r,g,b,alpha);
		
		// this can just be path.draw();
		//drawVectorDataExperimental(obj);
		obj->path.draw(0, 0); //drawVectorDataExperimental(obj);
		
		if(strokeWeight!="") ofSetLineWidth(1);
	}
	
	
	endRenderer();
	
	layers[layers.size()-1].objects.push_back(obj);
	
}

void ofxSVG::drawVectorDataExperimental(ofPath* object) {
	
	/*for(int k = 0; k < object->paths.size(); k++){
		vector<ofVec2f> *vec = object->paths.at(k);
		printf(" size %i ", vec->size());
		if(vec->size() < 1000) {
		ofBeginShape();
			for(int l = 0; l < vec->size(); l++) {
					ofVec2f pt = vec->at(l);
					ofVertex(pt.x, pt.y);
			}
		ofEndShape(false);
		}
	}*/
	
}

/*vector<ofPoint> ofxSVG::singleBezierToPtsWithResample(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float resampleDist){
	float   ax, bx, cx;
	float   ay, by, cy;
	float   t, t2, t3;
	float   x, y;
	
	vector <ofPoint> outPts;
	
	// polynomial coefficients
	cx = 3.0f * (x1 - x0);
	bx = 3.0f * (x2 - x1) - cx;
	ax = x3 - x0 - cx - bx;
	
	cy = 3.0f * (y1 - y0);
	by = 3.0f * (y2 - y1) - cy;
	ay = y3 - y0 - cy - by;
	
	float len = sqrt( (x3-x1)*(x3-x1) + (y3-y1)*(y3-y1) );
	
	int resolution = 2;
	
	if( resampleDist < 1 ){
		resampleDist = 1;
	}	
	
	if( len >= resampleDist * 2 ){
		resolution = (len / resampleDist );
	}else{
		resolution = 6;
	}
	
	for (int i = 0; i < resolution; i++){
		t 	=  ofMap(i+1, 0, resolution+1, 0, 1.0);
		t2 = t * t;
		t3 = t2 * t;
		x = (ax * t3) + (bx * t2) + (cx * t) + x0;
		y = (ay * t3) + (by * t2) + (cy * t) + y0;
		outPts.push_back(ofPoint(x, y));
	}
	
	return outPts;
}

void ofxSVG::drawVectorData(ofxSVGPath* path){
	ofBeginShape();								
	for(int k = 0; k < path->vectorData.size(); k++){
		if( path->vectorData[k].type == ofxSVGVector_Point ){
			ofVertex(path->vectorData[k].p.x, path->vectorData[k].p.y);
		}
		else if( path->vectorData[k].type == ofxSVGVector_BezierPoint ){
			ofBezierVertex(path->vectorData[k].c1.x, path->vectorData[k].c1.y, path->vectorData[k].c2.x, path->vectorData[k].c2.y, path->vectorData[k].p.x, path->vectorData[k].p.y);
		}
	}
	ofEndShape(false);
}

void ofxSVG::pathToVectorData(string pathStr, ofxSVGPath* obj){
	ofPoint drawPt;
	
	vector <string> commandStr = ofSplitString(pathStr, "*");
	
	for(int i = 0; i < commandStr.size(); i++){
		if( commandStr[i].length() ==  0 )continue;
		
		string currStr = commandStr[i];
		char command = currStr[0];
		
		currStr = currStr.substr(1, currStr.length()-1);
		
		//printf("[%c], curString is %s\n", command, currStr.c_str());
		
		vector <string> coordsStr = ofSplitString(currStr, "|");
		vector <float> coords;
		
		for(int i = 0; i < coordsStr.size(); i++){
			coords.push_back(ofToFloat(coordsStr[i]));
		}
		
		int numP = coords.size();
		
		if( ( command == 'm' || command == 'M' ) && numP >= 2){
			if( command == 'm'){
				drawPt.x += coords[0];
				drawPt.y += coords[1];				
			}else{
				drawPt.x = coords[0];
				drawPt.y = coords[1];
			}					
			//currShape.addPoint(drawPt.x, drawPt.y);
			obj->vectorData.push_back(ofxSVGPoint(drawPt.x, drawPt.y));
		}
		
		if( ( command == 'v' || command == 'V' ) && numP >= 1 ){
			
			for(int j = 0; j < numP; j++){
				
				if( command == 'v'){
					drawPt.y += coords[j+0];				
				}else{
					drawPt.y = coords[j+0];
				}					
				//currShape.addPoint(drawPt.x, drawPt.y);
				obj->vectorData.push_back(ofxSVGPoint(drawPt.x, drawPt.y));
			}
		}
		
		if( ( command == 'h' || command == 'H' ) && numP >= 1 ){
			
			for(int j = 0; j < numP; j++){
				
				if( command == 'h'){
					drawPt.x += coords[j+0];				
				}else{
					drawPt.x = coords[j+0];
				}					
				//currShape.addPoint(drawPt.x, drawPt.y);
				obj->vectorData.push_back(ofxSVGPoint(drawPt.x, drawPt.y));
			}
		}
		
		if( ( command == 'l' || command == 'L' )  && numP >= 2 ){
			
			for(int j = 0; j < numP; j+=2){
				
				if( command == 'l'){
					drawPt.x += coords[j+0];
					drawPt.y += coords[j+1];				
				}else{
					drawPt.x = coords[j+0];
					drawPt.y = coords[j+1];
				}					
				//currShape.addPoint(drawPt.x, drawPt.y);
				obj->vectorData.push_back(ofxSVGPoint(drawPt.x, drawPt.y));
			}
		}
		
		if( ( command == 'c' || command == 'C' )  && numP >= 6 ){
			
			for(int j = 0; j < numP; j+=6){
				
				float c1x, c1y;
				float c2x, c2y;
				
				if( command == 'c'){
					c1x = drawPt.x + coords[j+0];
					c1y = drawPt.y + coords[j+1];
					c2x = drawPt.x + coords[j+2];
					c2y = drawPt.y + coords[j+3];
					drawPt.x += coords[j+4];
					drawPt.y += coords[j+5];				
				}else{
					c1x = coords[j+0];
					c1y = coords[j+1];
					c2x = coords[j+2];
					c2y = coords[j+3];
					drawPt.x = coords[j+4];
					drawPt.y = coords[j+5];
				}					
				//currShape.addBezier(c1x, c1y, c2x, c2y, drawPt.x, drawPt.y);
				obj->vectorData.push_back(ofxSVGPoint(c1x, c1y, c2x, c2y, drawPt.x, drawPt.y));
			}
		}
		
		if( ( command == 's' || command == 'S' )  && numP >= 4 ){
			
			for(int j = 0; j < numP; j+=4){
				
				ofPoint prePoint = drawPt;
				
				ofPoint preHandle;
				if( obj->vectorData.size() && obj->vectorData.back().type == ofxSVGVector_BezierPoint ){
					preHandle = obj->vectorData.back().c2 - prePoint;
				}else{
					preHandle = drawPt - prePoint;
				}
				
				float c1x, c1y;
				float c2x, c2y;
				
				if( command == 's'){
					c1x = prePoint.x - preHandle.x;
					c1y = prePoint.y - preHandle.y;
					
					c2x = drawPt.x + coords[j+0];
					c2y = drawPt.y + coords[j+1];
					drawPt.x += coords[j+2];
					drawPt.y += coords[j+3];				
				}else{
					c1x = prePoint.x - preHandle.x;
					c1y = prePoint.y - preHandle.y;
					
					c2x = coords[j+0];
					c2y = coords[j+1];
					drawPt.x = coords[j+2];
					drawPt.y = coords[j+3];
				}					
				//currShape.addBezier(c1x, c1y, c2x, c2y, drawPt.x, drawPt.y);
				obj->vectorData.push_back(ofxSVGPoint(c1x, c1y, c2x, c2y, drawPt.x, drawPt.y));
			}
		}
		
	}
}

void ofxSVG::vectorDataToVertexs(ofxSVGPath* path, float resampleDist){	
	int numPts = path->vectorData.size();
	
	ofVec2f delta;
	
	vector <ofxSVGPoint> pts = path->vectorData;
	
	if( numPts >= 2 ){
		
		for(int k = 0; k < pts.size(); k++){					
			
			bool nAddPts = false;
			float len = 0.0;
			
			if( k > 0 ){
				delta		= pts[k].p - pts[k-1].p;
				len			= delta.length();
				if( len > resampleDist * 2 ){
					nAddPts = true;
				}
			}
			
			if( nAddPts ){
				
				if( pts[k].type == ofxSVGVector_BezierPoint && k > 0 ){
					
					
					vector <ofPoint> bezPts = singleBezierToPtsWithResample(pts[k-1].p.x,  pts[k-1].p.y,  
																			pts[k].c1.x,  pts[k].c1.y, 
																			pts[k].c2.x,  pts[k].c2.y,  
																			pts[k].p.x,  pts[k].p.y, resampleDist);
					
					for(int i = 0; i < bezPts.size(); i++){
						path->vertexs.push_back(bezPts[i]);
					}	
					
				}else{
					
					int numToAdd = ((int)(len / resampleDist))-1;
					
					for(int i = 0; i < numToAdd; i++){
						float pct = ofMap(i+1, 0, numToAdd+1, 0.0, 1.0);
						
						ofPoint iPnt = pts[k].p * pct + pts[k-1].p * (1.0-pct);
						path->vertexs.push_back(iPnt); 
					}
					
				}
				
			}
			
			path->vertexs.push_back(pts[k].p);
			}
			
		}	
}
*/

//-------------------------------------------------------------------------------------
///// ---------------------------------------------------------------------------
// BEGIN NEW
///// ---------------------------------------------------------------------------



///// ---------------------------------------------------------------------------
// END NEW
///// ---------------------------------------------------------------------------


ofPoint ofxSVG::posFromMatrix(string matrix){
    matrix = matrix.substr(7, matrix.length()-8);
    vector<string> matrixStrings = ofSplitString(matrix, " ");
    return ofVec2f(ofToFloat(matrixStrings[4]), ofToFloat(matrixStrings[5]));
}


float ofxSVG::rotFromMatrix(string matrix){
    int i = matrix.find("(");
    if(i!=-1){
        matrix = matrix.substr(i+1, matrix.length()-(i+2));
        vector<string> matrixStrings = ofSplitString(matrix, " ");
        return ofRadToDeg(atan2(ofToFloat(matrixStrings[3]),ofToFloat(matrixStrings[0])));
    }
    return 0.0f;
}

ofVec2f ofxSVG::scaleFromMatrix(string matrix) {
	matrix = matrix.substr(7, matrix.length()-8);
    vector<string> matrixStrings = ofSplitString(matrix, " ");
    return ofVec2f(ofToFloat(matrixStrings[1]), ofToFloat(matrixStrings[3]));	
}

float ofxSVG::scale(string scaleVal) {
    string floatVal = scaleVal.substr(scaleVal.find("("));
	floatVal = floatVal.substr(0, scaleVal.find(")"));
    return ofToFloat(floatVal);
}

//--------------------------------------------------------------
void ofxSVG::draw(){
    for(int i=0; i<layers.size(); i++){
        layers[i].draw();

    }
}
void ofxSVG::drawLayer(string layerName){
    for(int i=0; i<layers.size(); i++){
        if(layers[i].name == layerName){
            layers[i].draw();
            return;
        }
    }
}

void ofxSVG::drawLayer(int i){
    layers[i].draw();
}

//-------------------------------------------------------------------------
// begin save
//-------------------------------------------------------------------------


void ofxSVG::createRootSvg() {
	saveXml.addTag("svg");
	saveXml.addAttribute("svg", "xmlns", "http://www.w3.org/2000/svg", 0);
	saveXml.addAttribute("svg", "xmlns:xlink", "http://www.w3.org/1999/xlink", 0);
	saveXml.addAttribute("svg", "version", "1.1", 0);

}


void ofxSVG::addLayer(string layerName){

	if(saveXml.getValue("svg", "", 0) == "") {
		createRootSvg();
	}
	saveXml.pushTag("svg",0);
	saveXml.pushTag("g", 0);

}

void ofxSVG::saveToFile(string filename) {
	saveXml.saveFile(filename);
}

void ofxSVG::rect(float x, float y, float w, float h) {

	saveXml.pushTag("svg", 0);
	//saveXml.pushTag("rect", 0);// <rect
	saveXml.addTag("rect");
	saveXml.setAttribute("rect", "height", h, 0);
	saveXml.setAttribute("rect", "width", w, 0);
	saveXml.setAttribute("rect", "x", x, 0);
	saveXml.setAttribute("rect", "y", y, 0);
	saveXml.setAttribute("rect", "fill", currentAttributes["color"], 0);
	saveXml.setAttribute("rect", "stroke", currentAttributes["stroke"], 0);
	saveXml.setAttribute("rect", "stroke-width", currentAttributes["strokewidth"], 0);
	saveXml.popTag();

}
void ofxSVG::ellipse(float x, float y, float rx, float ry) {

	saveXml.addTag("ellipse");
	saveXml.setAttribute("ellipse", "ry", ry/2.0, 0);
	saveXml.setAttribute("ellipse", "rx", rx/2.0, 0);
	saveXml.setAttribute("ellipse", "cx", x, 0);
	saveXml.setAttribute("ellipse", "cy", y, 0);
	saveXml.setAttribute("ellipse", "fill", currentAttributes["color"], 0);
	saveXml.setAttribute("ellipse", "stroke", currentAttributes["stroke"], 0);
	saveXml.setAttribute("ellipse", "stroke-width", currentAttributes["strokewidth"], 0);
	saveXml.popTag();

}
void ofxSVG::circle(float x, float y, float r) {


	// need a way to get the current color, stroke settings...hmm.
	// probably something like: current_vals;
	saveXml.pushTag("svg", 0);
	saveXml.addTag("circle");
	saveXml.setAttribute("circle", "r", r/2.0, 0);
	saveXml.setAttribute("circle", "cx", x, 0);
	saveXml.setAttribute("circle", "cy", y, 0);
	saveXml.setAttribute("circle", "fill", currentAttributes["color"], 0);
	saveXml.setAttribute("circle", "stroke", currentAttributes["stroke"], 0);
	saveXml.setAttribute("circle", "stroke-width", currentAttributes["strokewidth"], 0);
	saveXml.popTag();
}



void ofxSVG::beginPolygon(){
	currentAttributes["drawingpolygon"] = "true";
	
	saveXml.pushTag("svg", 0);
	saveXml.addTag("polygon");
	saveXml.setAttribute("polygon", "fill", currentAttributes["color"], 0);
	saveXml.setAttribute("polygon", "stroke", currentAttributes["stroke"], 0);
	saveXml.setAttribute("polygon", "stroke-width", currentAttributes["strokewidth"], 0);

}
void ofxSVG::endPolygon(){
	currentAttributes["drawingpolygon"] = "false";
}

void ofxSVG::beginPath() {

	currentAttributes["drawingpath"] = "true";
	saveXml.pushTag("svg", 0);
	saveXml.addTag("path");
	saveXml.setAttribute("path", "fill", currentAttributes["color"], 0);
	saveXml.setAttribute("path", "stroke", currentAttributes["stroke"], 0);
	saveXml.setAttribute("path", "stroke-width", currentAttributes["strokewidth"], 0);
	
}
void ofxSVG::endPath() {
	currentAttributes["drawingpath"] = "false";
	
	string currentString;
	
	saveXml.pushTag("svg");
	stringstream s;
	currentString = saveXml.getAttribute("path", "d", "");
	if(currentString.length() > 1) { // i.e. has a path already been started?
		
		currentString += "Z";		
		saveXml.setAttribute("path", "d", s.str(), currentSaveNode);
	
	}
}

void ofxSVG::vertex(float x, float y){

	string currentPath;
	if(currentAttributes["drawingpath"] != "" && currentAttributes["drawingpath"] != "false") {
		saveXml.pushTag("svg", 0);
		currentPath = saveXml.getAttribute("path", "points", "", 0);
		stringstream s;
		s << x << "," << y << " ";
		currentPath+=s.str();
		saveXml.setAttribute("path", "points", currentPath, 0);
	}

	if(currentAttributes["drawingpolygon"] != "" && currentAttributes["drawingpolygon"] != "false") {
		saveXml.pushTag("svg", 0);
		currentPath = saveXml.getAttribute("polygon", "points", "", 0);
		stringstream s;
		s << x << "," << y << " ";
		currentPath+=s.str();
		saveXml.setAttribute("polygon", "points", currentPath, 0);
	}
}

// this is going to be a tough one
// because we'll have to calculate the next
// point
void ofxSVG::bezierVertex(float x0, float y0, float x1, float y1, float x2, float y2) {

	//
	string currentString;
	if(currentAttributes["drawingpath"] != "" && currentAttributes["drawingpath"] != "false") {
		saveXml.pushTag("svg");
		stringstream s;
		currentString = saveXml.getAttribute("path", "d", "");
		if(currentString.length() > 1) { // i.e. has a path already been started?

			s << " Q "<< x0 << " "<< x0 << " "<< y0 << " "<< x1 << " "<< y1 << " "<< x2 << " "<< y2 << " ";

		} else {
			s << " M "<< x0 << " "<< x0 << " "<< y0 << " "<< x1 << " "<< y1 << " "<< x2 << " "<< y2 << " ";

		}

		saveXml.setAttribute("path", "d", s.str(), currentSaveNode);
		currentAttributes["drawingpath"] = s.str();
	}
}
void ofxSVG::stroke(string colorHex, int weight) {
	currentAttributes["stroke"] = colorHex;

	stringstream s;
	s << weight;
	currentAttributes["strokewidth"] = s.str();


}

void ofxSVG::fill(string colorHex) {

	// figure out if it has an alpha
	int rgb = strtol(("0x"+colorHex.substr(1, colorHex.length()-1)).c_str(), NULL, 0);
	float r = (rgb >> 16) & 0xFF;
	float g = (rgb >> 8) & 0xFF;
	float b = (rgb) & 0xFF;
	
	currentAttributes["color"] = colorHex;
}

void ofxSVG::noFill() {

	currentAttributes["color"] = "";
}
void ofxSVG::noStroke(){
	currentAttributes["stroke"] = "";
}

void ofxSVG::setOpacity(float percent) {

	currentAttributes["opacity"] = "percent";
}

void ofxSVG::translate(float tx, float ty) {
	
	if(currentAttributes["matrix"] != "") {
		ofMatrix3x3 m = matrices[matrices.size() - 1];
		m.c += tx;
		m.f += ty;
		string s;
		stringFromMatrix(&s, matrices[matrices.size() - 1]);
		currentAttributes["matrix"] = s;
	}
}
void ofxSVG::rotate(float r) {

	if(currentAttributes["matrix"] != "") {
		ofMatrix3x3 m = matrices[matrices.size() - 1];
		m.a += cos(r);
		m.b += -sin(r);
		m.d += sin(r);
		m.e += cos(r);
		matrices.push_back(m);
	}
}

void ofxSVG::pushMatrix() {
	// how to get this to work?
	// one option is a vector of transform matrices that are just summed up
	// another I suppose would be to see if SVG supports using multiple
	// matrices, but that 
	if(currentAttributes["matrix"] != "") {
		
		ofMatrix3x3 m;		
		m = matrices[matrices.size() - 1]; // build on the old one
		matrices.push_back(m);// this just copies the matrix over but it should be fine
	}
	
}
void ofxSVG::popMatrix() {
	if(currentAttributes["matrix"] != "") {
		
		matrices.pop_back();
		if(matrices.size() == 0) {
			currentAttributes["matrix"] = "";
		} else {
			string s;
			stringFromMatrix(&s, matrices[matrices.size() - 1]);
			currentAttributes["matrix"] = s;

		}
	}
	
}

string ofxSVG::createAttribute(string element, ...) { // va_args ftw!

	int i, numberArgs;
	va_list vl;
	va_start(vl,numberArgs);
	va_arg(vl,int);
	for(i = 0; i<numberArgs; i++) {
		
		
	}
}

void ofxSVG::matrixFromString(string smat, ofMatrix3x3 mat) {
	
	size_t i, j;
	string spaceconst = " ";
	
	i = smat.find(spaceconst, 0);
	while(i != string::npos) {
		
		mat[j] = atof(smat.substr(i, smat.find(spaceconst, i) - i).c_str());
		i = smat.find(spaceconst, i);
		j++;
	}
}

void ofxSVG::stringFromMatrix(string* smat, ofMatrix3x3 mat) {
	int i = 0;
	stringstream s;
	s << "matrix(";
	while(i < 9) {
		
		s << mat[i] << " ";
		i++;
	}
	s << ")";
	//	int length = s.str().length();
	smat->insert(0, s.str());
}


