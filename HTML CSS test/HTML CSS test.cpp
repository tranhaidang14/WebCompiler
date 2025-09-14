#include "pch.h"
#include "CppUnitTest.h"
#include "../HTMLParser/html_parser.h"
#include "../HTMLParser/css_parser.h"

#include <string>
#include <iostream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace HTMLCSStest
{
	TEST_CLASS(HTMLCSStest)
	{
	public:
		// Perfect html
		TEST_METHOD(HTML_Test1_PerfectCode)
		{
			std::string html =
				"<!DOCTYPE html>\n"
				"<html lang=\"en\">\n"
				"<head>\n"
				"    <meta charset=\"UTF-8\">\n"
				"    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
				"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
				"    <title>Document</title>\n"
				"</head>\n"
				"<body>\n"
				"    <h1 id=\"header\" class=\"title\">Hello World</h1>\n"
				"    <p>This is a paragraph.</p>\n"
				"    <img src=\"image.jpg\" alt=\"Image\">\n"
				"</body>\n"
				"</html>";
			
			HTML_ParseTree tree = HTMLParser::parse(html, HTML_Doctype::HTML5);
			Assert::AreEqual(tree.query("*").size(), size_t(13));
			Assert::AreEqual(HTMLParser::getError().code, HTML_OK);
		}

		// Not closed tags
		TEST_METHOD(HTML_Test2_UnclosedTag)
		{
			std::string html =
				"<!DOCTYPE html>\n"
				"<html lang=\"en\">\n"
				"<head>\n"
				"    <meta charset=\"UTF-8\">\n"
				"    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
				"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
				"    <title>Document</title>\n"
				"</head>\n"
				"<body>\n"
				"    <h1 id=\"header\" class=\"title\">Hello World</h1>\n"
				"    <p>This is a paragraph.</p>\n"
				"    <img src=\"image.jpg\" alt=\"Image\">\n"
				"</html>";

			HTML_ParseTree tree = HTMLParser::parse(html, HTML_Doctype::HTML5);

			Assert::AreNotEqual(HTMLParser::getError().code, HTML_OK);
		}

		// Attributes test
		TEST_METHOD(HTML_Test2_Attributes)
		{
			std::string HTML = "<html lang=\"en\"><img src=\"hello\" lazy_load></html>";

			HTML_ParseTree tree = HTMLParser::parse(HTML, HTML_Doctype::HTML5);
			std::shared_ptr<HTML_Node> html = tree.query("html")[0], img = tree.query("img")[0];

			Assert::AreEqual("en", html->attributes.get_attribute("lang").c_str());
			Assert::AreEqual("true", img->attributes.get_attribute("lazy_load").c_str());
		}
	
	
		TEST_METHOD(CSS_Test1_PerfectCode) {
			std::string css =
				".a .b > div {\n"
				"color: blue;\n"
				"};";

			CSS parsed = CSSParser::parse(css);
		}
	};
}
