#include "html_parser.h"
#include "query_selector.h"

#include<queue>
#include<sstream>
#include<iostream>

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>


template<typename T, typename ...Opts>
bool any_of(T val, Opts ...opts)
{
	return (... || (val == opts));
}

template<typename T, typename ...Opts>
bool tag_is(T val, Opts ...opts)
{
	return (... || (val.find(opts) == 0));
}
static void ltrim(std::string& s) {
	int i;
	for (i = 0; i < s.length(); ++i) {
		if (!std::isspace(s[i])) break;
	}
	if (i > 0) {
		s = s.substr(i);
	}
}

#pragma region attribute
HTML_Attributes::HTML_Attributes(string raw) {
	if (raw.empty()) return;
	std::cout << "Parsing Attribute: " << raw << "\n";
	ltrim(raw);
	std::vector<string> tokens;
	
	string token = "";
	bool is_string_started = false;
	char ch = raw[0];
	// tokenize the raw string into tokens
	for (int i = 0; i < raw.length(); ++i, ch = raw[i]) {
		if (ch == '=') {
			if (is_string_started) {
				token += ch;
				continue;
			}

			if (token.empty()) {
				std::cout << ch << " ";
				HTML_Error = { 0, ATTRIBUTE_ERROR, string("Syntax error in attributes: ") + raw};
				return;
			}
			tokens.push_back(token);
			tokens.push_back("=");
			token = "";
		}
		else if(std::isspace(ch)) {
			if (token.empty()) continue;
			tokens.push_back(token);
			token = "";
		}
		else if (ch == '\"') {
			if (is_string_started) {
				is_string_started = false;
				tokens.push_back(token);
				token = "";
			}
			else {
			if (!token.empty()) {
				HTML_Error = { 0, ATTRIBUTE_ERROR,  string("Syntax error in attributes: ") + raw };
				return;
			}
				is_string_started = true;
			}
		}
		else {
			token += ch;
		}
	}
	
	if (token != "") tokens.push_back(token);

	// Parse it to attributes
	string key = "", value = "";
	bool flag = false;
	for (int i = 0; i < tokens.size(); ++i) {
		if (key.empty()) key = tokens[i];
		else if (tokens[i] == "=") flag = true;
		else {
			if (flag) {
				value = tokens[i];

				set_attribute(key, value);
				key = value = "";
				flag = false;
			}
			else {
				// attribute without value
				set_attribute(key, "true");
				key = "";
				--i; //preprocess this as key in the next iteration
			}
		}
	}
	if (key != "") set_attribute(key, "true");
}

bool HTML_Attributes::set_attribute(string key, string value) {
	std::cout << key << "=" << value;
	attributes[key] = value;
	return true;
};
string HTML_Attributes::get_attribute(string key) {
	if (attributes.find(key) != attributes.end()) {
		return attributes[key];
	}

	HTML_Error = { 0, ATTRIBUTE_ERROR, "Attribute not found" };
	return "";
};
bool HTML_Attributes::remove_attribute(string key) {
	attributes.erase(key);
	return true;
};

#pragma endregion

#pragma region node
bool HTML_Node::add_child(std::shared_ptr<HTML_Node> child) {
	childrens.push_back(child);
	return true;
};

bool HTML_Node::remove_child(std::string query_selector) {
	HTML_NodeList nodes_to_remove;
	
	QuerySelector query(query_selector);
    query.get_matched(shared_from_this(), 0, nodes_to_remove);
	
	for (auto& node : nodes_to_remove) {
		if (!node->get_parent()) continue; // root node
		auto parent = node->get_parent();
		// push all children of the node that will be removed to back of the vector then erase it
		parent->childrens.erase(
			std::remove(parent->childrens.begin(), parent->childrens.end(), node), 
			parent->childrens.end()
		);
	}

	return true;
};

void HTML_Node::push_all_children_to_queue(std::queue<std::shared_ptr<HTML_Node>>& q, bool direct) {
	for (auto& child : childrens) {
		q.push(child);
		if (!direct) child->push_all_children_to_queue(q);
	}
}
#pragma endregion

#pragma region parse_tree
HTML_NodeList HTML_ParseTree::query(string query_selector) const {
	HTML_NodeList result;
	QuerySelector query(query_selector);
	QuerySelector(query_selector).get_matched(root, 0, result);

	return result;
}

void HTML_ParseTree::traverse_query(string query_selector, std::function<void(std::shared_ptr<HTML_Node> node)> cb) const {
	HTML_NodeList nodes = query(query_selector);

	for (auto& node : nodes) {
		cb(node);
	}
}
#pragma endregion

#pragma region parser
namespace HTMLParser {
	std::queue<HTML_Token> tokenize(string html_content) {
		std::queue<HTML_Token> tokens;
		HTML_Token temp_token = { OPENING_TAG, "" };

		int i = 0;
		while (i < html_content.length()) {
			switch (html_content[i]) {
			case '<':
				switch (temp_token.type) {
					// an OPENING_TAG is pre-started
				case OPENING_TAG:
					++i;
					break;
					// in case it's just an < of html content
				case CONTENT:
					ltrim(temp_token.value);
					// but only if it's not empty
					if (temp_token.value != "") {
						tokens.push({ CONTENT, temp_token.value });
					}
					// pre-start a new OPENING_TAG token
					temp_token = { OPENING_TAG, "" };
					++i;
					break;
				}
				break;
			case '>':

				ltrim(temp_token.value);
				if (tag_is<string, string>(temp_token.value, "img", "meta")) {
					temp_token.type = SELF_CLOSING_TAG;
				}

				// close current tag: opening_tag, closing_tag, self_closing_tag, doctype 
				if (any_of<HTML_TokenType, HTML_TokenType>(temp_token.type, OPENING_TAG, CLOSING_TAG, SELF_CLOSING_TAG, DOCTYPE)) {
					tokens.push(temp_token);
					temp_token = { CONTENT, "" };
				}

				else if (temp_token.type == DOCTYPE) {
					if (temp_token.value.substr(0, 2) == "--") {
						// it's a comment
						temp_token.type = COMMENT;
						// remove the starting --
						temp_token.value = temp_token.value.substr(2);
						// remove the ending --
						if (temp_token.value.length() >= 2 && temp_token.value.substr(temp_token.value.length() - 2) == "--") {
							temp_token.value = temp_token.value.substr(0, temp_token.value.length() - 2);
						}
						tokens.push(temp_token);
						temp_token = { CONTENT, "" };
					}
					else {
						// it's a doctype
						tokens.push(temp_token);
						temp_token = { CONTENT, "" };
					}
				}

				else if (temp_token.type == CONTENT ) {
					temp_token.value += html_content[i];
				}
				++i;
				break;
			case '/':
				// if it's treated as an opening tag and / is found, it's a closing/self-closing tag
				if (temp_token.type == OPENING_TAG) {
					ltrim(temp_token.value);
					if (temp_token.value.empty()) {
						temp_token.type = CLOSING_TAG;
						++i;
						break;
					}
					temp_token.type = SELF_CLOSING_TAG;
					++i;
				}
				break;

				// Can be doctype, comment or content
			case '!':
				if (temp_token.type == OPENING_TAG && temp_token.value == "") {
					temp_token.type = DOCTYPE;
					++i;
					break;
				}
				temp_token.value += html_content[i];
				++i;
				break;
				// add content
			default:
				temp_token.value += html_content[i];
				++i;
			}
		}
		tokens.push({ EOF_TOKEN, "" });
		return tokens;
	}

	HTML_ParseTree parse(string html_content, HTML_Doctype doctype) {
		std::queue<HTML_Token> tokens = tokenize(html_content);

		HTML_ParseTree tree(doctype);
		std::shared_ptr<HTML_Node> current_scope_node(tree.getRoot());

		// Declare variables outside the switch to avoid E0546
		string tag_name;
		HTML_Attributes attributes;
		std::shared_ptr<HTML_Node> temp_node;

		while (!tokens.empty()) {
			if (HTML_Error.code != HTML_OK) return tree;
			HTML_Token token = tokens.front(); tokens.pop();
			switch (token.type) {
			case EOF_TOKEN:
				break;
			case OPENING_TAG:
				tag_name = token.value.substr(0, token.value.find(' '));
				token.value.erase(0, tag_name.length());

				attributes = HTML_Attributes(token.value);
				temp_node = std::make_shared<HTML_Node>(tag_name, attributes, current_scope_node);

				current_scope_node->add_child(temp_node);
				current_scope_node = temp_node;
				break;
			case CLOSING_TAG:
				if (current_scope_node->get_tag() == token.value) {
					current_scope_node = current_scope_node->get_parent();
					break;
				}
				else {
					HTML_Error = { 0, PARSING_ERROR, "Mismatched closing tag: " + token.value };
				}
				break;
			case SELF_CLOSING_TAG:
				tag_name = token.value.substr(0, token.value.find(' '));
				token.value.erase(0, tag_name.length());

				attributes = HTML_Attributes(token.value);
				temp_node = std::make_shared<HTML_Node>(tag_name, attributes, current_scope_node);

				current_scope_node->add_child(temp_node);
				break;
			case CONTENT:
				if (!token.value.empty()) {
					attributes = HTML_Attributes();
					temp_node = std::make_shared<HTML_Node>("#text", attributes, current_scope_node);
					current_scope_node->add_child(temp_node);
				}
				break;
			case COMMENT:
				break;
			case DOCTYPE:
				break;
			}
		}

		if (current_scope_node != tree.getRoot()) {
			HTML_Error = { 0, PARSING_ERROR, "Some tags are not closed" };
		}
		return tree;
	}

	HTML_Except getError() {
		return HTML_Error;
	}
};

#pragma endregion
