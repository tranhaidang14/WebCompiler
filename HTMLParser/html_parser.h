#pragma once
#include<string>
#include<vector>
#include<unordered_map>
#include<memory>
#include<queue>
#include<functional>

using std::string;
class HTML_Node;
class HTML_NodeList : public std::vector<std::shared_ptr<HTML_Node>> {
public:
	std::shared_ptr<HTML_Node> operator[](int index) {
		if (this->size() == 0) return nullptr;
		
		return this->at(index);
	}
};

struct HTML_Except {
	int line, code;
	string message;
};

static HTML_Except HTML_Error = { 0, 0, "OK" };

constexpr int HTML_OK = 0x0;
constexpr int HTML_SYNTAX_ERROR = 0x1;
constexpr int ATTRIBUTE_ERROR = 0x2;
constexpr int PARSING_ERROR = 0x3;

enum HTML_Doctype {
	HTML5,
	HTML4,
	XHTML,
	UNKNOWN_DOCTYPE
};


/**
* Class representing HTML attributes like class, id, style, etc.
*/
class HTML_Attributes {
	std::unordered_map<string, string> attributes;

public:
	HTML_Attributes() {};
	// is attributes in the form of raw string like: key1="value1" key2=123 key3, it's obtained from the tag name of an opening tag token that are exclude the tag name
	HTML_Attributes(string raw);

	bool set_attribute(string key, string value);
	bool remove_attribute(string key);

	string get_attribute(string key);
};


/**
* Class representing a node in the HTML parse tree
*/
class HTML_Node : public std::enable_shared_from_this<HTML_Node> {
	std::shared_ptr<HTML_Node> parent = nullptr;
	HTML_NodeList childrens;

	string tag;

public:
	HTML_Attributes attributes;

	HTML_Node(string tag) : tag(tag) {};
	HTML_Node(string tag, HTML_Attributes attributes) : tag(tag), attributes(attributes) {};
	HTML_Node(string tag, HTML_Attributes attributes, std::shared_ptr<HTML_Node> parent) : tag(tag), attributes(attributes), parent(parent) {};

	bool add_child(std::shared_ptr<HTML_Node> child);
	bool remove_child(std::string query_selector); // query selector is like CSS selector

	void push_all_children_to_queue(std::queue<std::shared_ptr<HTML_Node>>& q, bool direct = false);

	std::shared_ptr<HTML_Node> get_parent() { return std::shared_ptr<HTML_Node>(parent); }
	string get_tag() { return tag; }
};

class HTML_ParseTree {
	HTML_Doctype doctype;
	std::shared_ptr<HTML_Node> root;

public:
	HTML_ParseTree(HTML_Doctype doctype) : doctype(doctype), root(std::make_shared<HTML_Node>("DOCUMENT")) {};
	std::shared_ptr<HTML_Node>& getRoot() { return root; }

	HTML_NodeList query(string query_selector) const;
	void traverse_query(string query_selector, std::function<void(std::shared_ptr<HTML_Node> node)> cb) const;
};


enum HTML_TokenType {
	OPENING_TAG,
	CLOSING_TAG,
	SELF_CLOSING_TAG,
	CONTENT,
	COMMENT,
	DOCTYPE,
	EOF_TOKEN
};

struct HTML_Token {
	HTML_TokenType type;
	string value;
};

namespace HTMLParser {
	// If you care about error, after getting the parsed tree, you should check for getError().code
	HTML_ParseTree parse(string html_content, HTML_Doctype doctype);
	// The line number aren't implemented yet
	HTML_Except getError();
};