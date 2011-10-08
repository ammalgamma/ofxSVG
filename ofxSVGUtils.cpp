/*
 *  ofxSVGUtils.cpp
 *  emptyExample
 *
 *  Created by base on 02/10/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxSVGUtils.h"

errs ofxSVGUtils::err = NO_ERR;

const char* ofxSVGUtils::strCur;
const char* ofxSVGUtils::strEnd = 0;
const char* ofxSVGUtils::functionName = 0;
size_t ofxSVGUtils::functionLen = 0;

float ofxSVGUtils::d[6] = {0, 0, 0, 0, 0, 0};
size_t ofxSVGUtils::d_count = 0;
string ofxSVGUtils::str = "";