//
//  main.cpp
//  GamePlusPlus
//
//  Created by Uli Kusterer on 23.05.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include "token.hpp"
#include "file_access.hpp"
#include "fake_filesystem.hpp"
#include "parse_error.hpp"


using namespace std;
using namespace fake;


void skip_line_breaks(vector<token> &tokens, vector<token>::iterator &currToken, const string &fname)
{
	while (currToken != tokens.end() && currToken->type == lineBreak) {
		cout << endl << "#line " << currToken->lineNumber << " \"" << fname << "\"" << endl;
		++currToken;
	}
}


void parse_for_instance_names(vector<token> &tokens, const string &fname)
{
	vector<token>::iterator currToken = tokens.begin();
	int nestingLevel = 0;
	
	while(currToken != tokens.end()) {
		if (nestingLevel == 0 && currToken->is_identifier("object")) {
			++currToken;
			skip_line_breaks(tokens, currToken, fname);
			
			if (currToken == tokens.end()) {
				parse_error err(currToken->startOffset, currToken->endOffset, fname);
				err.message << "Expected class name after \"object\", found end of file.";
				throw err;
			} else if (currToken->type != identifier) {
				parse_error err(currToken->startOffset, currToken->endOffset, fname);
				err.message << "Expected class name after \"object\", found \"" << currToken->text << "\".";
				throw err;
			}
			string objectName = currToken->text;
			
			++currToken;
			skip_line_breaks(tokens, currToken, fname);
			
			if (currToken == tokens.end()) {
				parse_error err(currToken->startOffset, currToken->endOffset, fname);
				err.message << "Expected superclass specification or start of object after object definition, found end of file.";
				throw err;
			} else if (currToken->type != colonOperator && currToken->type != openCurlyBracket) {
				parse_error err(currToken->startOffset, currToken->endOffset, fname);
				err.message << "Expected superclass specification or start of object after object definition, found \"" << currToken->text << "\".";
				throw err;
			}

			string baseClassName = "object_class";
			if (currToken->type == colonOperator) {
				++currToken;
				skip_line_breaks(tokens, currToken, fname);
				
				if (currToken == tokens.end()) {
					parse_error err(currToken->startOffset, currToken->endOffset, fname);
					err.message << "Expected superclass name after \":\", found end of file.";
					throw err;
				} else if (currToken->type != identifier) {
					parse_error err(currToken->startOffset, currToken->endOffset, fname);
					err.message << "Expected superclass name after \":\", found \"" << currToken->text << "\".";
					throw err;
				}
				
				baseClassName = currToken->text;
				baseClassName.append("_class");
				
				++currToken;
				skip_line_breaks(tokens, currToken, fname);
			}
			cout << "extern class " << baseClassName << " &" << objectName << ";" << endl;
		} else if(currToken->type == openCurlyBracket) {
			++nestingLevel;
			++currToken;
		} else if(currToken->type == closeCurlyBracket) {
			--nestingLevel;
			++currToken;
		} else if(currToken->type == lineBreak) {
			skip_line_breaks(tokens, currToken, fname);
		} else {
			++currToken;
		}
	}
}


void parse_for_instances(vector<token> &tokens, const string &fname)
{
	vector<token>::iterator currToken = tokens.begin();
	int nestingLevel = 0;
	bool isInObject = false;
	string currObjectName;
	string currBaseClassName;
	size_t currObjectLineNum = 0;
	
	while(currToken != tokens.end()) {
		if (nestingLevel == 0 && currToken->is_identifier("object")) {
			currObjectLineNum = currToken->lineNumber;
			++currToken;
			skip_line_breaks(tokens, currToken, fname);

			if (currToken == tokens.end()) {
				parse_error err(currToken->startOffset, currToken->endOffset, fname);
				err.message << "Expected class name after \"object\", found end of file.";
				throw err;
			} else if (currToken->type != identifier) {
				parse_error err(currToken->startOffset, currToken->endOffset, fname);
				err.message << "Expected class name after \"object\", found \"" << currToken->text << "\".";
				throw err;
			}
			currObjectName = currToken->text;
			
			++currToken;
			skip_line_breaks(tokens, currToken, fname);

			if (currToken == tokens.end()) {
				parse_error err(currToken->startOffset, currToken->endOffset, fname);
				err.message << "Expected superclass specification or start of object after object definition, found end of file.";
				throw err;
			} else if (currToken->type != colonOperator && currToken->type != openCurlyBracket) {
				parse_error err(currToken->startOffset, currToken->endOffset, fname);
				err.message << "Expected superclass specification or start of object after object definition, found \"" << currToken->text << "\".";
				throw err;
			}
			
			currBaseClassName = "object_class";
			if (currToken->type == colonOperator) {
				++currToken;
				skip_line_breaks(tokens, currToken, fname);

				if (currToken == tokens.end()) {
					parse_error err(currToken->startOffset, currToken->endOffset, fname);
					err.message << "Expected superclass name after \":\", found end of file.";
					throw err;
				} else if (currToken->type != identifier) {
					parse_error err(currToken->startOffset, currToken->endOffset, fname);
					err.message << "Expected superclass name after \":\", found \"" << currToken->text << "\".";
					throw err;
				}
				
				currBaseClassName = currToken->text;
				currBaseClassName.append("_class");
				
				++currToken;
				skip_line_breaks(tokens, currToken, fname);
			}

			cout << "class " << currObjectName << "_class : " << currBaseClassName;
			isInObject = true;
		} else if(currToken->type == openCurlyBracket) {
			++nestingLevel;
			++currToken;
			cout << " {";
		} else if(currToken->type == closeCurlyBracket) {
			--nestingLevel;
			++currToken;
			
			if (nestingLevel == 0 && isInObject) {
				isInObject = false;
				if (currToken == tokens.end() || currToken->type != semicolon) {
					parse_error err(currToken->startOffset, currToken->endOffset, fname);
					err.message << "Expected semicolon name after object definition's \"}\".";
					throw err;
				}
				++currToken;
				cout << "};" << endl;
				cout << "#line " << currObjectLineNum << "\"" << fname << "\"" << endl;
				cout << "class " << currBaseClassName << " &" << currObjectName << " = " << currObjectName << "_class();" << endl;
			} else {
				cout << " }";
			}
		} else if(currToken->type == lineBreak) {
			skip_line_breaks(tokens, currToken, fname);
		} else if (currToken->type == stringLiteral) {
			cout << " \"" << currToken->text << "\"";
			++currToken;
		} else {
			cout << " " << currToken->text;
			++currToken;
		}
	}
}


int main(int argc, const char * argv[])
{
	try {
		filesystem::path				containingFolderPath(filesystem::path(argv[1]).parent_path());
		filesystem::directory_iterator	currFile(containingFolderPath);
		
		for( ; currFile != filesystem::directory_iterator(); ++currFile )
		{
			filesystem::path	fpath( (*currFile).path() );
			string				fname( fpath.filename().string() );
			if( fname.length() > 0 && fname[0] == '.' )
				continue;
			if( fname.rfind(".gmp") != fname.length() -4 )
				continue;
			
			string script(file_contents(fname));
			vector<token> tokens = token::tokenize(script);
			
			if (fname.compare(argv[1]) == 0) {
//				cout << "FILE: " << argv[1] << endl;
				token::debug_print(tokens, cout);
				
				parse_for_instances(tokens, fname);
			} else {
//				cout << "PEER: " << fname.c_str() << endl;
				token::debug_print(tokens, cout);
				
				parse_for_instance_names(tokens, fname);
			}
		}
		
		cout << "done." << endl;
	} catch(const exception& err) {
		cerr << err.what() << endl;
	}
	
	return 0;
}
