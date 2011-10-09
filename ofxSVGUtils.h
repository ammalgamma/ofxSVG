/*
 *  ofxSVGUtils.h
 *  
 *
 *  Created by base on 26/09/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma mark once

#include "ofMain.h"
#include "ofxSVGConstants.h"

enum errs {
	//tbd
	NO_ERR, ERR
};

class ofxSVGUtils {
public:
	//err_t parseTransform(TransformF& dst, const StringW& str)
	static int parseTransform(ofMatrix4x4& _dst, const string& _str)
	{
		str = _str;
		
		strCur = str.c_str();
		strEnd = strCur + str.size();
		const char* functionName;
		size_t functionLen;
		
		//dst.clear();
		startTransformParse(_dst);
		return endTransformParse(err);
	};
	
	static void skew( ofMatrix4x4& mat, float x, float y) {
		
		float p0 = tan(x);
		float p1 = tan(y);
		
		if (abs(p0) < 0.00001) { p0 = 0.0; }
		if (abs(p1) < 0.00001) { p1 = 0.0; }
		
		ofMatrix4x4 skew(1.0, p1, 0.0, 0.0,
						 p0, 1.0, 0.0, 0.0,
						 0.0, 0.0, 1.0, 0.0,
						 0.0, 0.0, 0.0, 0.0);
		
		mat.postMult(skew);
	}
	
	void matrixFromString(string &smat, ofMatrix3x3 &mat) {
		
		size_t i, j;
		string spaceconst = " ";
		
		i = smat.find(spaceconst, 0);
		while(i != string::npos) {
			
			mat[j] = atof(smat.substr(i, smat.find(spaceconst, i) - i).c_str());
			i = smat.find(spaceconst, i);
			j++;
		}
	}
	
	static void matrixFromNum(float a, float b, float c, float d, float e, float f, ofMatrix3x3 &mat) {
		mat[0] = a;
		mat[3] = b;
		mat[1] = a;
		mat[4] = b;
		mat[2] = a;
		mat[5] = b;
		mat[8] = 1.f;
	}
	
	static void multSVGMatTo4x4(ofMatrix3x3 &three, ofMatrix4x4 &four) {
		four(0, 0) *= three[0];
		four(0, 1) *= three[1];
		four(0, 2) *= three[2];
		four(1, 0) *= three[3];
		four(1, 1) *= three[4];
		four(1, 2) *= three[5];
	}
	
private:
	
	static errs err;
	
	static const char* strCur;
	static const char* strEnd;
	static const char* functionName;
	static size_t functionLen;
	
	static float d[6];
	static size_t d_count;
	static string str;

	static void startTransformParse(ofMatrix4x4& dst) {
		// Skip spaces.
		for (;;)
		{
			if (strCur == strEnd) endTransformParse(err);
			else if (isAsciiSpace(strCur)) strCur++;
			else break;
		}
		
		// Parse function name.
		functionName = strCur;
		for (;;)
		{
			if (strCur == strEnd) endTransformParse(err);
			else if (isAsciiLetter(strCur)) strCur++;
			else break;
		}
		functionLen = (size_t)(strCur - functionName);
		
		// Parse '('.
		if (strCur[0] != '(') endTransformParse(err);
		strCur++;
		
		// Parse arguments.
		d_count = 0;
		for (;;)
		{
			if (strCur == strEnd) endTransformParse(err);
			
			// Parse number.
			size_t end;
			//float* dst,  const char* str, size_t length, char decimalPoint = '.'
			if (stringToDouble(&d[d_count++], strCur, (size_t)(strEnd - strCur), '.', &end) != 0)
			{
				endTransformParse(err);
			}
			
			strCur += end;
			
			// Skip ',' and move to position of the next digit.
			bool commaParsed = false;
			for (;;)
			{
				if (strCur == strEnd) endTransformParse(err);
				
				if (isAsciiSpace(strCur))
				{
					strCur++;
				}
				else if (strCur[0] == ',')
				{
					strCur++;
					if (commaParsed) endTransformParse(err);
					commaParsed = true;
				}
				else if (strCur[0] == ')')
				{
					strCur++;
					if (commaParsed) endTransformParse(err);
					finishTransformParse(dst, err);
				}
				else if (isAsciiDigit(strCur[0]) || strCur[0] == '-' || strCur[0] == '+')
				{
					break;
				}
				else
				{
					endTransformParse(err);
				}
			}
			
			if (d_count == 6) endTransformParse(err);
		}
	};

	static void finishTransformParse(ofMatrix4x4& dst, errs err) {
		// matrix() function.
		if (functionLen == 6 && stringEquals(functionName, "matrix", 6))
		{
			if (d_count != 6) endTransformParse(err);
			
			//d[0], d[1], d[2], d[3], d[4], d[5];
			ofMatrix3x3 mat;
			matrixFromNum(d[0], d[1], d[2], d[3], d[4], d[5], mat);
			multSVGMatTo4x4(mat, dst);
			
		}
		// translate() function.
		else if (functionLen == 9 && stringEquals(functionName, "translate", 9))
		{
			if (d_count != 1 && d_count != 2) endTransformParse(err);
			// If ty is not provided, it's assumed to be zero.
			if (d_count == 1) d[1] = 0.0f;
			dst.translate(ofVec2f(d[0], d[1]));
		}
		// scale() function.
		else if (functionLen == 5 && stringEquals(functionName, "scale", 5))
		{
			if (d_count != 1 && d_count != 2) endTransformParse(err);
			// If sy is not provided, it's assumed to be equal to sx.
			if (d_count == 1) d[1] = d[0];
			dst.scale(ofVec2f(d[0], d[1]));
		}
		// rotate() function.
		else if (functionLen == 6 && stringEquals(functionName, "rotate", 6))
		{
			if (d_count != 1 && d_count != 3) endTransformParse(err);
			
			if (d_count == 3) dst.translate(ofVec2f(d[1], d[2]));
			
			dst.rotate(ofDegToRad(d[0]), 1.0, 1.0, 0);
			
			if (d_count == 3) dst.translate(ofVec2f(-d[1], -d[2]));
		}
		// skewX() function.
		else if (functionLen == 5 && stringEquals(functionName, "skewX", 5))
		{
			if (d_count != 1) endTransformParse(err);
			skew(dst, ofDegToRad(d[0]), 0.0f);
		}
		// skewY() function.
		else if (functionLen == 5 && stringEquals(functionName, "skewY", 5))
		{
			if (d_count != 1) endTransformParse(err);
			skew(dst, 0.0f, ofDegToRad(d[0]));
		}
		else
		{
			endTransformParse(err);
		}
		
		// Skip spaces.
		for (;;)
		{
			if (strCur == strEnd) break;
			
			if (isAsciiSpace( strCur))
				strCur++;
			else
				startTransformParse(dst);
		}
	};

	static int endTransformParse(errs err) {
		return err;
	};

	static bool stringEquals(const char* a, const char* b, size_t length)
	{
		for (size_t i = 0; i < length; i++)
		{
			if (a[i] != b[i])
				return false;
		}
		return true;
	};

	static bool stringToDouble( const string& str, double& d  ) {

		d = ::atof(str.c_str());
		return true;
	};

	//float* dst,  const char* str, size_t length, char decimalPoint = '.', size_t* pEnd = NULL, uint32_t* pFlags = NULL)
	static bool stringToDouble( float* dst,  const char* str, size_t length, char decimalPoint = '.', size_t* pEnd = NULL  ) {
		
		int count = 0;
		while( count < length && str[count] && (isAsciiDigit( str[count]) || str[count] == decimalPoint)) {
			++count;
		}
		char copy[count];
		memcpy(copy, str, count);
		*dst = float (::atof(copy));
	};

	static const uint8_t asciiCTypeData[256];
	
	static inline bool isAsciiBlank(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_BLANK) != 0; };
	static inline bool isAsciiCntrl(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_PRINT) == 0; };
	static inline bool isAsciiDigit(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_DIGIT) != 0; };
	static inline bool isAsciiGraph(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_GRAPH) != 0; };
	static inline bool isAsciiLetter(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_ALPHA) != 0; };
	static inline bool isAsciiLower(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_LOWER) != 0; };
	static inline bool isAsciiNumlet(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_ALNUM) != 0; };
	static inline bool isAsciiPrint(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_PRINT) != 0; };
	static inline bool isAsciiPunct(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_PUNCT) != 0; };
	static inline bool isAsciiSpace(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_SPACE) != 0; };
	static inline bool isAsciiUpper(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_UPPER) != 0; };
	static inline bool isAsciiXDigit(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_XDIGIT) != 0; };
	static inline bool isAsciiXLetter(const char *c) { return (asciiCTypeData[*c] & ofxSVGConstant::ASCII_CLASS_XALPHA) != 0; };
	
	static inline bool isAsciiBlank(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_BLANK) != 0; };
	static inline bool isAsciiCntrl(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_PRINT) == 0; };
	static inline bool isAsciiDigit(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_DIGIT) != 0; };
	static inline bool isAsciiGraph(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_GRAPH) != 0; };
	static inline bool isAsciiLetter(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_ALPHA) != 0; };
	static inline bool isAsciiLower(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_LOWER) != 0; };
	static inline bool isAsciiNumlet(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_ALNUM) != 0; };
	static inline bool isAsciiPrint(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_PRINT) != 0; };
	static inline bool isAsciiPunct(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_PUNCT) != 0; };
	static inline bool isAsciiSpace(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_SPACE) != 0; };
	static inline bool isAsciiUpper(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_UPPER) != 0; };
	static inline bool isAsciiXDigit(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_XDIGIT) != 0; };
	static inline bool isAsciiXLetter(const char c) { return (asciiCTypeData[c] & ofxSVGConstant::ASCII_CLASS_XALPHA) != 0; };

};