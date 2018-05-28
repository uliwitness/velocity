//
//  main.cpp
//  GamePlusPlus
//
//  Created by Uli Kusterer on 23.05.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "token.hpp"
#include "file_access.hpp"
#include "fake_filesystem.hpp"
#include "parse_error.hpp"


#if 0
#define LINE_INSTR(_file,_filename,_line) 			do {} while(0)
#else
#define LINE_INSTR(_file,_filename,_line) 			_file << "#line " << _line << "\"" << _filename << "\"" << endl
#endif


using namespace std;
using namespace fake;


void skip_line_breaks(vector<token> &tokens, vector<token>::iterator &currToken, const string &fname, ostream &destHeaderFile, ostream &destSourceFile)
{
	while (currToken != tokens.end() && currToken->type == lineBreak) {
		destHeaderFile << endl;
		destSourceFile << endl;
		LINE_INSTR(destHeaderFile, fname, currToken->lineNumber);
		LINE_INSTR(destSourceFile, fname, currToken->lineNumber);
		++currToken;
	}
}


void parse_for_instances(vector<token> &tokens, const string &fname, ostream &headerDestFile, ostream &sourceDestFile)
{
	vector<token>::iterator currToken = tokens.begin();
	int nestingLevel = 0;
	bool isInObject = false;
	string currObjectName;
	string currBaseClassName;
	size_t currObjectLineNum = 0;
	
	vector<token> sourceSignatureTokens;
	
	while(currToken != tokens.end()) {
		if (currToken->is_identifier("func")) {
			++currToken;
			
			while (currToken != tokens.end() && currToken->type != openCurlyBracket) {
				if (currToken->type == openParenthesis) { // Found start of param list, previous token must be function name.
					string currClassName(currObjectName);
					currClassName.append("_class");
					// Insert foo_class:: before it so source file has full function name:
					sourceSignatureTokens.insert(sourceSignatureTokens.end() - 1, token(identifier, currClassName));
					sourceSignatureTokens.insert(sourceSignatureTokens.end() - 1, token(scopeResolutionOperator, "::"));
				}
				sourceSignatureTokens.push_back(*currToken);
				headerDestFile << " " << currToken->text_for_code();
				++currToken;
			}
			
			for (const token& t : sourceSignatureTokens) {
				sourceDestFile << " " << t.text_for_code();
			}
			
			if (currToken != tokens.end() && currToken->type == openCurlyBracket) {
				sourceDestFile << endl << "{" << endl;
				headerDestFile << ";" << endl;
				
				++nestingLevel;
				++currToken;
			}
		} else if (currToken->is_identifier("prop")) {
			++currToken;
			
			while (currToken != tokens.end() && currToken->type != semicolon) {
				headerDestFile << " " << currToken->text_for_code();
				++currToken;
			}
			
			if (currToken != tokens.end() && currToken->type == semicolon) {
				headerDestFile << ";" << endl;
				
				++currToken;
			}
		} else if (nestingLevel == 0 && currToken->is_identifier("object")) {
			currObjectLineNum = currToken->lineNumber;
			++currToken;
			skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);

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
			skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);

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
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);

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
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
			}

			headerDestFile << "struct " << currObjectName << "_class : " << currBaseClassName << endl;
			isInObject = true;
		} else if(currToken->type == openCurlyBracket) {
			++nestingLevel;
			++currToken;
			LINE_INSTR(headerDestFile, fname, currToken->lineNumber);
			if (nestingLevel == (0 + 1) && isInObject) {
				headerDestFile << " {" << endl;
			} else {
				sourceDestFile << " {" << endl;
			}
		} else if(currToken->type == closeCurlyBracket) {
			--nestingLevel;
			++currToken;
			
			if (nestingLevel == 0 && isInObject) {
				isInObject = false;
				if (currToken == tokens.end() || currToken->type != semicolon) {
					parse_error err(currToken->startOffset, currToken->endOffset, fname);
					err.message << "Expected semicolon after object definition's \"}\".";
					throw err;
				}
				++currToken;
				headerDestFile << "};" << endl;
				LINE_INSTR(headerDestFile, fname, currObjectLineNum);
				headerDestFile << endl << "extern " << currObjectName << "_class " << currObjectName << ";" << endl;
				LINE_INSTR(headerDestFile, fname, currObjectLineNum);
				sourceDestFile << endl << currObjectName << "_class " << currObjectName << ";" << endl;
				
				currObjectName = "";
			} else {
				sourceDestFile << " }";
			}
		} else if(currToken->type == lineBreak) {
			skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
		} else {
			sourceDestFile << " " << currToken->text_for_code();
			++currToken;
		}
	}
}


int main(int argc, const char * argv[])
{
	try {
		filesystem::path				containingFolderPath(filesystem::path(argv[1]).parent_path());
		
		cout << "note: Parent folder " << containingFolderPath.string() << endl;
		
		filesystem::directory_iterator	currFile(containingFolderPath);
		filesystem::path 				inputPath(argv[1]);
		string							sourceDestName(inputPath.filename().string());
		const char*						suffix = ".gmp";
		off_t suffixPos = sourceDestName.rfind(suffix);
		if (suffixPos != string::npos) {
			sourceDestName.erase(suffixPos, strlen(suffix));
		}
		string							headerDestName(sourceDestName);
		sourceDestName.append(".cpp");
		headerDestName.append(".hpp");

		cout << "note: Writing system classes to object_class.hpp" << endl;
		ofstream						baseclassfile("object_class.hpp", ofstream::out | ofstream::trunc);
		baseclassfile << "#pragma once" << endl << endl
		<< "#include <iostream>" << endl
		<< "#include <string>" << endl << endl
		<< "using namespace std;" << endl << endl
		<< "class object_class {" << endl
		<< "};" << endl
		<< endl
		<< "class room_class : object_class {" << endl
		<< "};" << endl;

		cout << "note: output to " << sourceDestName << " and " << headerDestName << endl;

		ofstream						headerDestFile(headerDestName, ofstream::out | ofstream::trunc);
		ofstream						sourceDestFile(sourceDestName, ofstream::out | ofstream::trunc);
		headerDestFile << "/* This file was auto-generated using GamePlusPlus from the file \"" << argv[1] << "\"." << endl
		<< "   Do not edit this file, all changes will be overwritten." << endl
		<< "   Edit the original file instead. */" << endl << endl
		<< "#pragma once" << endl << endl
		<< "#include \"object_class.hpp\"" << endl;

		sourceDestFile << "/* This file was auto-generated using GamePlusPlus from the file \"" << argv[1] << "\"." << endl
		<< "   Do not edit this file, all changes will be overwritten." << endl
		<< "   Edit the original file instead. */" << endl
		<< "#include \"" << filesystem::path(headerDestName).filename() << "\"" << endl;

		stringstream peerIncludes;
		stringstream headerBody;
		
		for( ; currFile != filesystem::directory_iterator(); ++currFile )
		{
			filesystem::path	fpath( (*currFile).path() );
			string				fname( fpath.filename().string() );
			if( fname.length() > 0 && fname[0] == '.' )
				continue;
			if( fname.rfind( ".gmp" ) != fname.length() -4 )
				continue;
			
			cout << "note: processing " << fpath.string() << endl;
			
			if (filesystem::equivalent(fpath, filesystem::path(argv[1]))) {
				string script(file_contents(fpath.string()));
				vector<token> tokens = token::tokenize(script);
				
				cout << "note: FILE: " << argv[1] << endl;
				//token::debug_print(tokens, cout);
				
				parse_for_instances(tokens, fname, headerBody, sourceDestFile);
			} else {
				cout << "note: PEER: " << fname.c_str() << endl;
				
				string				headername( fname );
				headername.erase( fname.length() -4, 4 );
				headername.append( ".hpp" );
				
				peerIncludes << "#include \"" << headername << "\"" << endl;
			}
			
		}

		headerDestFile << peerIncludes.str() << headerBody.str();

		cout << "note: done." << endl;
	} catch(const exception& err) {
		cerr << "error: " << err.what() << endl;
	}
	
	return 0;
}
