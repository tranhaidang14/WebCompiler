#include "html_parser.h"
#include<iostream>
#include<queue>

int main() {
	HTML_ParseTree tree = HTMLParser::parse("<!DOCTYPE html><html class = \"a\"><head><title>Test a > 3 < / title>< / head><body><h1>Hello, World!<!-- This is a comment -->< / h1> <h1></h1>< / body>< / html>", HTML_Doctype::HTML5);
	tree.getRoot()->remove_child("h1");

	std::cout << tree.query("h1").size() << std::endl;
	return 0;
}
