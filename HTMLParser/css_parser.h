#pragma once
#include<string>
#include<unordered_map>
#include<functional>

// Forward declaration
class QuerySelector;

using std::string;

struct CSS_Except {
	int line, code;
	string message;
};

static CSS_Except CSS_Error;

const int CSS_OK = 0x0;

/*
* **********
* * Styles *
* **********
*/

struct CSS_Units {
	float value;
	enum{percent, px, em, rem} type;
};

struct RGB {
	uint8_t r, g, b;
};

template <typename T>
struct DirectionalProperties {
	T top, bottom, right, left;
};

// Style object store all css properties. Each DOM element will have one of its all
struct Styles {
	CSS_Units width, height;
	DirectionalProperties<CSS_Units> margin, padding;
	
	RGB background_color, color;
	DirectionalProperties<RGB> border_color;
	
	string font_family;
	CSS_Units font_size;

	enum {BLOCK, INLINE, FLEX, GRID} display;
};

// represent a css file/tag content
class CSS {
	std::unordered_map<QuerySelector, Styles> css;
	
public:
	void loopthrough(std::function<void(QuerySelector, Styles)> cb);
};

namespace CSS_Parser {
	CSS parse(string raw);
	CSS_Except getError();
}
