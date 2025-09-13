#pragma once
#include "html_parser.h"

#include<string>

using std::string;

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
