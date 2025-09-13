#include "css_parser.h"

#include <sstream>
#include <queue>
#include <iostream>

void QuerySelector::query(string raw) {
	size = 1;
	tokens[0] = { QueryTokenType::COMBINATOR, " " };

	this->raw = raw;
	// tokenize the raw string into tokens

	std::stringstream s(raw);
	string token;
	while (s >> token) {
		if (token == ">" || token == "~" || token == "+") {
			tokens[size++] = { QueryTokenType::COMBINATOR, token };
			continue;
		}

		if (size > 0 && tokens[size - 1].type != QueryTokenType::COMBINATOR) {
			tokens[size++] = { QueryTokenType::COMBINATOR, " " };
		}

		tokens[size++] = { QueryTokenType::IDENTIFIER, token };
	}
}

void QuerySelector::get_matched(std::shared_ptr<HTML_Node> node, int token_index, HTML_NodeList& list) {
	if (!node) return;
	if (token_index >= size) return;
	
	string identifier;
	QueryToken token = tokens[token_index];

	if (token.type == QueryTokenType::IDENTIFIER) {
		// id
		if (token.value[0] == '#') {
			identifier = node->attributes.get_attribute("id");
		}
		// class
		else if (token.value[0] == '.') {
			identifier = node->attributes.get_attribute("class");
		}
		// tag
		else {
			identifier = node->get_tag();
		}

		if (identifier != token.value.substr(token.value[0] == '#' || token.value[0] == '.') && token.value != "*") {
			return;
		}

		if (token_index == size - 1) {
			// last token, add to list
			list.push_back(node);
			return;
		}

		// Here the identifier is matched
		get_matched(node, token_index + 1, list);
	}

	else if (token.type == QueryTokenType::COMBINATOR) {
		std::queue<std::shared_ptr<HTML_Node>> q;
		if (token.value == " ") {
			// queue all children and grandchildren
			node->push_all_children_to_queue(q, false);
		}
		else if (token.value == ">") {
			// queue all direct children
			node->push_all_children_to_queue(q, true);
		}

		while (!q.empty()) {
			std::shared_ptr<HTML_Node> child = q.front();
			q.pop();
			get_matched(child, token_index + 1, list);
		}
	}
}


