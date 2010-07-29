#include "svgPathParser.h"

static int currentToken;

void svgPathParser::parse(const char** attr)
{
	nbuf = 0;
	const char* s;
	char cmd;
	float args[10];
	int nargs;
	int rargs;
	float cpx, cpy, cpx2, cpy2;
	const char* tmp[4];
	char closedFlag;
	int i;
	char item[64];
	
		
	s = attr[0];
	
	svgResetPath();
	closedFlag = 0;
	nargs = 0;
	
	while (*s)
	{
		s = getNextPathItem(s, item);
		if (!*item) break;
		
		if (isnum(item[0]))
		{
			if (nargs < 10)
				args[nargs++] = (float)atof(item);
			if (nargs >= rargs)
			{
				switch (cmd)
				{
					case 'm':
					case 'M':
					case 'l':
					case 'L':
						pathLineTo(&cpx, &cpy, args, (cmd == 'm' || cmd == 'l') ? 1 : 0);
						break;
					case 'H':
					case 'h':
						pathHLineTo(&cpx, &cpy, args, cmd == 'h' ? 1 : 0);
						break;
					case 'V':
					case 'v':
						pathVLineTo(&cpx, &cpy, args, cmd == 'v' ? 1 : 0);
						break;
					case 'C':
					case 'c':
						pathCubicBezTo(&cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
						break;
					case 'S':
					case 's':
						pathCubicBezShortTo(&cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
						break;
					case 'Q':
					case 'q':
						pathQuadBezTo(&cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
						break;
					case 'T':
					case 't':
						pathQuadBezShortTo(&cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
						break;
					default:
						if (nargs >= 2)
						{
							cpx = args[nargs-2];
							cpy = args[nargs-1];
						}
						break;
				}
				nargs = 0;
			}
		}
		else
		{
			cmd = item[0];
			rargs = getArgsPerElement(cmd);
			if (cmd == 'M' || cmd == 'm')
			{
				// Commit path.	
				// TODO get all the transform/translate/etc data from the parent?
				// or just return out of this and handle in the parent?
				//path = new vector<ofxVec2f*>();
				// Start new subpath.
				//svgResetPath();
				closedFlag = 0;
				nargs = 0;
				cpx = 0; cpy = 0;
			}
			else if (cmd == 'Z' || cmd == 'z')
			{
				closedFlag = 1;
				// Commit path.
				//path = new vector<ofxVec2f*>();
				// Start new subpath.
				svgResetPath();
				closedFlag = 0;
				nargs = 0;
			}
		}
	}
	/*
	// Commit path.
	path = new vector<ofxVec2f*>();
	*/	
}


void svgPathParser::cubicBez(float x1, float y1, float cx1, float cy1, float cx2, float cy2, float x2, float y2)
{
	cubicBezRec(x1,y1, cx1,cy1, cx2,cy2, x2,y2, 0); 
	svgPathPoint(x2, y2);
}

void svgPathParser::quadBezRec( float x1, float y1, float x2, float y2, float x3, float y3, int level)
{
	float x12,y12,x23,y23,x123,y123,d;
	
	if (level > 12) return; // don't go too deep
	
	x12 = (x1+x2)*0.5f;                
	y12 = (y1+y2)*0.5f;
	x23 = (x2+x3)*0.5f;
	y23 = (y2+y3)*0.5f;
	x123 = (x12+x23)*0.5f;
	y123 = (y12+y23)*0.5f;
	
	d = distPtSeg(x123, y123, x1,y1, x3,y3);
	if (level > 0 && d < 1000) // tol*tol)
	{
		svgPathPoint(x123, y123);
		return;
	}
	
	quadBezRec(x1,y1, x12,y12, x123,y123, level+1); 
	quadBezRec(x123,y123, x23,y23, x3,y3, level+1); 
}

void svgPathParser::quadBez(float x1, float y1, float cx, float cy, float x2, float y2)
{
	quadBezRec(x1,y1, cx,cy, x2,y2, 0); 
	svgPathPoint(x2, y2);
}


void svgPathParser::pathCubicBezTo(float* cpx, float* cpy, float* cpx2, float* cpy2, float* args, int rel)
{
	
	float x1, y1, x2, y2, cx1, cy1, cx2, cy2;
	
	x1 = *cpx;
	y1 = *cpy;
	if (rel)
	{
		cx1 = *cpx + args[0];
		cy1 = *cpy + args[1];
		cx2 = *cpx + args[2];
		cy2 = *cpy + args[3];
		x2 = *cpx + args[4];
		y2 = *cpy + args[5];
	}
	else
	{
		cx1 = args[0];
		cy1 = args[1];
		cx2 = args[2];
		cy2 = args[3];
		x2 = args[4];
		y2 = args[5];
	}
	
	cubicBez(x1,y1, cx1,cy1, cx2,cy2, x2,y2);
	
	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

void svgPathParser::pathCubicBezShortTo(float* cpx, float* cpy,	float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx1, cy1, cx2, cy2;
	
	x1 = *cpx;
	y1 = *cpy;
	if (rel)
	{
		cx2 = *cpx + args[0];
		cy2 = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	}
	else
	{
		cx2 = args[0];
		cy2 = args[1];
		x2 = args[2];
		y2 = args[3];
	}
	
	cx1 = 2*x1 - *cpx2;
	cy1 = 2*y1 - *cpy2;
	
	cubicBez(x1,y1, cx1,cy1, cx2,cy2, x2,y2);
	
	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

void svgPathParser::pathQuadBezTo(float* cpx, float* cpy, float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	
	x1 = *cpx;
	y1 = *cpy;
	if (rel)
	{
		cx = *cpx + args[0];
		cy = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	}
	else
	{
		cx = args[0];
		cy = args[1];
		x2 = args[2];
		y2 = args[3];
	}
	
	quadBez(x1,y1, cx,cy, x2,y2);
	
	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

void svgPathParser:: svgPathParser::pathQuadBezShortTo(float* cpx, float* cpy, float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	
	x1 = *cpx;
	y1 = *cpy;
	if (rel)
	{
		x2 = *cpx + args[0];
		y2 = *cpy + args[1];
	}
	else
	{
		x2 = args[0];
		y2 = args[1];
	}
	
	cx = 2*x1 - *cpx2;
	cy = 2*y1 - *cpy2;
	
	quadBez(x1,y1, cx,cy, x2,y2);
	
	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

void svgPathParser::cubicBezRec(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int level)
{
	float x12,y12,x23,y23,x34,y34,x123,y123,x234,y234,x1234,y1234;
	float d;
	
	if (level > 12) return;
	
	x12 = (x1+x2)*0.5f;
	y12 = (y1+y2)*0.5f;
	x23 = (x2+x3)*0.5f;
	y23 = (y2+y3)*0.5f;
	x34 = (x3+x4)*0.5f;
	y34 = (y3+y4)*0.5f;
	x123 = (x12+x23)*0.5f;
	y123 = (y12+y23)*0.5f;
	x234 = (x23+x34)*0.5f;
	y234 = (y23+y34)*0.5f;
	x1234 = (x123+x234)*0.5f;
	y1234 = (y123+y234)*0.5f;
	
	d = distPtSeg(x1234, y1234, x1,y1, x4,y4);
	if (level > 0 && d < 1000)//tol*tol)
	{
		svgPathPoint(x1234, y1234);
		return;
	}
	
	cubicBezRec(x1,y1, x12,y12, x123,y123, x1234,y1234, level+1); 
	cubicBezRec(x1234,y1234, x234,y234, x34,y34, x4,y4, level+1); 
}

void svgPathParser::pathLineTo(float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
	{
		*cpx += args[0];
		*cpy += args[1];
	}
	else
	{
		*cpx = args[0];
		*cpy = args[1];
	}
	svgPathPoint(*cpx, *cpy);
}

void svgPathParser::pathHLineTo(float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpx += args[0];
	else
		*cpx = args[0];
	svgPathPoint(*cpx, *cpy);
}

void svgPathParser::pathVLineTo(float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpy += args[0];
	else
		*cpy = args[0];
	svgPathPoint(*cpx, *cpy);
}

float svgPathParser::distPtSeg(float x, float y, float px, float py, float qx, float qy)
{
	float pqx, pqy, dx, dy, d, t;
	pqx = qx-px;
	pqy = qy-py;
	dx = x-px;
	dy = y-py;
	d = pqx*pqx + pqy*pqy;
	t = pqx*dx + pqy*dy;
	if (d > 0) t /= d;
	if (t < 0) t = 0;
	else if (t > 1) t = 1;
	dx = px + t*pqx - x;
	dy = py + t*pqy - y;
	return dx*dx + dy*dy;
}

int svgPathParser::getArgsPerElement(char cmd)
{
	switch (tolower(cmd))
	{
		case 'v':
		case 'h':
			return 1;
		case 'm':
		case 'l':
		case 't':
			return 2;
		case 'q':
		case 's':
			return 4;
		case 'c':
			return 6;
		case 'a':
			return 7;
	}
	return 0;
}

const char* svgPathParser::getNextPathItem(const char* s, char* it)
{
	int i = 0;
	it[0] = '\0';
	// Skip white spaces and commas
	while (*s && (isspace(*s) || *s == ',')) s++;
	if (!*s) return s;
	if (*s == '-' || *s == '+' || isnum(*s))
	{
		while (*s == '-' || *s == '+')
		{
			if (i < 63) it[i++] = *s;
			s++;
		}
		while (*s && *s != '-' && *s != '+' && isnum(*s))
		{
			if (i < 63) it[i++] = *s;
			s++;
		}
		it[i] = '\0';
	}
	else
	{
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}
	return s;
}


void svgPathParser::svgPathPoint(float x, float y) {
	printf(" new point %f %f length %i \n", x, y, path->size());
	ofxVec2f v(x, y);
	path->push_back(v);
}

int svgPathParser::isnum(char c)
{
	return strchr("0123456789+-.eE", c) != 0;
}

void svgPathParser::svgResetPath() 
{
	// here's where your problem is.
	
	vector<ofxVec2f> *newv = new vector<ofxVec2f>();
	newv = path;
	int i, j;
	/*for (i = 0; i<newv->size(); i++) {
		printf(" %f %f ", newv->at(i).x, newv->at(i).y);
	}*/
	
	pathInstance->paths.push_back(newv);
	printf(" last added path size %i \n", pathInstance->paths.at(pathInstance->paths.size()-1)->size());
	path = new vector<ofxVec2f>();
	/*
	for (i = 0; i<pathInstance->paths.size(); i++) {
		for (j = 0; j<pathInstance->paths.at(i)->size(); j++) {
			printf(" %f %f ", pathInstance->paths.at(i)->at(j).x, pathInstance->paths.at(i)->at(j).y);
		}
	}*/
	
	//	path = &vp;
	npos = 0;
}
