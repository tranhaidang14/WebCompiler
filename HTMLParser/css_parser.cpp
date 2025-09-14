#include "css_parser.h"
#include<vector>
#include<iostream>
#include "query_selector.h"

namespace CSSParser {
#pragma region tokenize
	static bool tokenize_phase1(string raw, std::vector<string>& tokens) {
		string token = "";
		char& ch = raw[0];
		bool block_started = false;
		int line = 1;

		for (int i = 0; i < raw.length(); ++i) {
			ch = raw[i];

			if (ch == '\n') {
				line++;
				token += '\n';
				continue;
			}

			if (ch == '{') {
				if (block_started) {
					CSS_Error = { line, CSS_SYNTAX_ERROR, "Nested block"};
					return false;
				}

				if (token == "") {
					CSS_Error = { line, CSS_SYNTAX_ERROR, "Block without query selector" };
					return false;
				}
			
				tokens.push_back(token);

				token = "";
				block_started = true;
				continue;
			}
			
			if (ch == '}') {
				if (block_started) {
					tokens.push_back(token);
					block_started = false;
					token = "";

					continue;
				}
				CSS_Error = { line, CSS_SYNTAX_ERROR, "Block didn't start but ended" };
				return false;
			}

			token += ch;
		}
	}

	static void tokenize_phase2(string raw) {
		std::vector<string> tokens;
		tokenize_phase1(raw, tokens);
	}
#pragma endregion

	CSS parse(string raw) {
		CSS css;
		tokenize_phase2(raw);

		return css;
	}

	CSS_Except getError() {
		return CSS_Error;
	}
}

