#pragma once
#include "html_parser.h"

#include<string>
#include<unordered_map>

using std::string;

struct CSS_Except {
	int line, code;
	string message;
};

CSS_Except CSS_Error;

const int CSS_OK = 0x0;

/*
* ******************
* * QUERY SELECTOR *
* ******************
*/
enum QueryTokenType {
	IDENTIFIER, // tag, .class, #id
	COMBINATOR, // space, >, +, ~
	//ATTRIBUTE, // [attr=value] DO LATER
	//PSEUDO_CLASS, // :hover, :nth-child(n) DO LATER
	//PSEUDO_ELEMENT // ::before, ::after DO LATER
};

struct QueryToken {
	QueryTokenType type = IDENTIFIER;
	string value = "";
};

class QuerySelector {
	static const int max_size = 10;
	string raw;
	QueryToken tokens[max_size]; // max 10 tokens for now

	int size = 0;
public:
	QuerySelector(string raw) { query(raw); }

	void query(string raw);
	void get_matched(std::shared_ptr<HTML_Node> node, int token_index, HTML_NodeList& list);
	
	int get_token_count() { return size; }
};

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
