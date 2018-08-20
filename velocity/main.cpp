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
#define LINE_INSTR(_file,_filename,_line) 			_file << "#line " << _line << " \"" << _filename << "\"" << endl
#endif


using namespace std;
using namespace fake;


class templatetypeinfo
{
public:
	string name;
	bool isConst = false;
};


class classinfo
{
public:
	string classname;
	vector<templatetypeinfo> templateparams;
	string baseclass;
	vector<templatetypeinfo> basetemplateparams;
	bool isbuiltin = false;
};


void skip_line_breaks(vector<token> &tokens, vector<token>::iterator &currToken, const string &fname, ostream &headerDestFile, ostream &sourceDestFile)
{
	while (currToken != tokens.end() && currToken->type == lineBreak) {
		headerDestFile << endl;
		sourceDestFile << endl;
		LINE_INSTR(headerDestFile, fname, currToken->lineNumber);
		LINE_INSTR(sourceDestFile, fname, currToken->lineNumber);
		++currToken;
	}
}


bool go_next_token_if_possible(vector<token> &tokens, vector<token>::iterator &currToken, const string &fname)
{
	if (currToken == tokens.end()) {
		return false;
	}
	
	vector<token>::iterator prevToken = currToken;
	++currToken;
	
	if (currToken == tokens.end()) {
		currToken = prevToken;
		return false;
	}
	
	return true;
}


void go_next_token(vector<token> &tokens, vector<token>::iterator &currToken, const string &fname)
{
	if (currToken == tokens.end()) {
		parse_error err(tokens.back().endOffset, tokens.back().endOffset, fname);
		err.message << "Unexpected end of file.";
		throw err;
	}
	if (!go_next_token_if_possible(tokens, currToken, fname)) {
		parse_error err(currToken->startOffset, currToken->endOffset, fname);
		err.message << "Unexpected end of file after \"" << currToken->text << "\".";
		throw err;
	}
}


string token_text(vector<token> &tokens, const vector<token>::iterator &currToken, const string &fname)
{
	if (currToken->type != identifier) {
		parse_error err(currToken->startOffset, currToken->endOffset, fname);
		err.message << "Expected identifier, found \"" << currToken->text << "\".";
		throw err;
	}
	
	return currToken->text;
}


void parse_template_params(vector<token> &tokens, vector<token>::iterator &currToken, vector<templatetypeinfo>& theTemplateParams, const string &fname, ostream &headerDestFile, ostream &sourceDestFile)
{
	if (currToken->type == lessThanOperator) {
		go_next_token(tokens, currToken, fname); // <
		
		while (currToken->type != greaterThanOperator) {
			templatetypeinfo	tti;
			
			if (currToken->is_identifier("const")) {
				tti.isConst = true;
				go_next_token(tokens, currToken, fname);
			}
			
			tti.name = token_text(tokens, currToken, fname);
			theTemplateParams.push_back(tti);
			
			go_next_token(tokens, currToken, fname);
			
			if (currToken->type == commaOperator) {
				go_next_token(tokens, currToken, fname);
				continue;
			}
		}
		
		go_next_token(tokens, currToken, fname); // >
	}
}


string parse_type(vector<token> &tokens, vector<token>::iterator &currToken, vector<classinfo> classInfos, const string &fname, ostream &headerDestFile, ostream &sourceDestFile)
{
	if (currToken == tokens.end() || currToken->type != identifier) {
		return "";
	}
	
	string typeStr;
	
	if (currToken->is_identifier("unsigned")) {
		typeStr.append("unsigned ");
		
		go_next_token(tokens, currToken, fname);
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
	} else if (currToken->is_identifier("signed")) {
		typeStr.append("signed ");
		
		go_next_token(tokens, currToken, fname);
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
	}
	
	if (currToken->type == identifier && currToken->is_identifier("short")) {
		typeStr.append("short ");
		
		go_next_token(tokens, currToken, fname);
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
	} else if (currToken->type == identifier && currToken->is_identifier("long")) {
		typeStr.append("long ");
		
		go_next_token(tokens, currToken, fname);
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
		
		if (currToken->type == identifier && currToken->is_identifier("long")) {
			typeStr.append("long ");
			
			go_next_token(tokens, currToken, fname);
			skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
		}
	}
	
	if (currToken->type == identifier && currToken->is_identifier("int")) {
		typeStr.append("int ");
		
		go_next_token(tokens, currToken, fname);
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
	} else if (currToken->type == identifier && currToken->is_identifier("char")) {
		typeStr.append("char ");
		
		go_next_token(tokens, currToken, fname);
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
	}

	if (typeStr.length() == 0) {
		string currTypeName = currToken->text;
		for (classinfo &currInfo : classInfos) {
			if (currTypeName.compare(currInfo.classname) == 0) {
				typeStr = currTypeName;
				
				go_next_token(tokens, currToken, fname);
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
				break;
			}
		}
	}
	
	if (currToken->type == lessThanOperator) {
		int nestingDepth = 1;
		typeStr.append("<");

		go_next_token(tokens, currToken, fname);
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
		
		while (true) {
			if (currToken == tokens.end()) {
				parse_error err(tokens.back().endOffset, tokens.back().endOffset, fname);
				err.message << "Missing \">\" at end of template type.";
				throw err;
			}
			
			if (currToken->type == lessThanOperator) {
				++nestingDepth;
			} else if (currToken->type == greaterThanOperator) {
				--nestingDepth;
			}
			
			typeStr.append(" ");
			typeStr.append(currToken->text);
			
			go_next_token(tokens, currToken, fname);
			skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);

			if (nestingDepth == 0) {
				break;
			}
		}
	}
	
	while (currToken != tokens.end() && (currToken->type == multiplyOperator || currToken->type == andOperator)) {
		typeStr.append(currToken->text);
		
		go_next_token(tokens, currToken, fname);
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
	}
	
	return typeStr;
}


void parse_for_instances(vector<token> &tokens, const string &fname, ostream &headerDestFile, ostream &sourceDestFile)
{
	vector<token>::iterator currToken = tokens.begin();
	vector<classinfo> classInfos;
	
	classInfos.push_back( classinfo({ "void", {}, "", {}, true }) );

	while(currToken != tokens.end()) {
		skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
		
		if (currToken == tokens.end()) {
			break;
		}
		
		if (currToken->is_identifier("class")) {
			skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
			go_next_token(tokens, currToken, fname);
			
			classInfos.push_back(classinfo());
			
			classinfo &theInfo = classInfos.back();
			theInfo.classname = token_text(tokens, currToken, fname);

			skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
			go_next_token(tokens, currToken, fname);
			
			parse_template_params(tokens, currToken, theInfo.templateparams, fname, headerDestFile, sourceDestFile);
			
			skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
			if (currToken == tokens.end()) {
				parse_error err(tokens.back().endOffset, tokens.back().endOffset, fname);
				err.message << "Unexpected end of file.";
				throw err;
			}
			
			if (currToken->type == colonOperator) {
				go_next_token(tokens, currToken, fname); // :
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
				
				theInfo.baseclass = token_text(tokens, currToken, fname);
				go_next_token(tokens, currToken, fname);
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);

				parse_template_params(tokens, currToken, theInfo.basetemplateparams, fname, headerDestFile, sourceDestFile);
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
			}

			if (currToken->type == semicolon) { // Forward.
				headerDestFile << "class " << theInfo.classname;
				if (theInfo.templateparams.size() > 0) {
					headerDestFile << "<";
					bool isFirst = true;
					for (templatetypeinfo &currTypeInfo : theInfo.templateparams) {
						if (isFirst) {
							isFirst = false;
						} else {
							headerDestFile << ", ";
						}
						headerDestFile << currTypeInfo.name;
					}
					headerDestFile << ">";
				}
				headerDestFile << ";" << endl;
				
				go_next_token(tokens, currToken, fname); // ;
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
			} else if (currToken->type == openCurlyBracket) { // Implementation.
				if (theInfo.templateparams.size() > 0) {
					headerDestFile << "template<";
					bool isFirst = true;
					for (templatetypeinfo &currTypeInfo : theInfo.templateparams) {
						if (isFirst) {
							isFirst = false;
						} else {
							headerDestFile << ", ";
						}
						headerDestFile << currTypeInfo.name;
					}
					headerDestFile << ">" << endl;
				}
				headerDestFile << "class " << theInfo.classname;
				if (theInfo.baseclass.length() > 0) {
					headerDestFile << " : " << theInfo.baseclass;
					if (theInfo.basetemplateparams.size() > 0) {
						headerDestFile << "<";
						bool isFirst = true;
						for (templatetypeinfo &currTypeInfo : theInfo.basetemplateparams) {
							if (isFirst) {
								isFirst = false;
							} else {
								headerDestFile << ", ";
							}
							headerDestFile << currTypeInfo.name;
						}
						headerDestFile << ">";
					}
				}
				headerDestFile << endl << "{" << endl;
				
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
				go_next_token(tokens, currToken, fname); // {
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);

				while (currToken->type != closeCurlyBracket) {
					string returnType = parse_type(tokens, currToken, classInfos, fname, headerDestFile, sourceDestFile);

					string memberName = token_text(tokens, currToken, fname);
					
					go_next_token(tokens, currToken, fname);
					skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
					
					if (currToken->type == openParenthesis) { // Method
						go_next_token(tokens, currToken, fname); // (
						skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
						
						go_next_token(tokens, currToken, fname); // )
						skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
						
						if(currToken->type != semicolon) {
							parse_error err(currToken->endOffset, currToken->endOffset, fname);
							err.message << "Expected \";\", found \"" << currToken->text << "\".";
							throw err;
						}

						go_next_token(tokens, currToken, fname); // ;
						skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
						
						headerDestFile << "\t" << returnType << " " << memberName << "();" << endl;
						if (theInfo.templateparams.size() > 0) {
							sourceDestFile << "template<";
							bool isFirst = true;
							for (templatetypeinfo &currTypeInfo : theInfo.templateparams) {
								if (isFirst) {
									isFirst = false;
								} else {
									sourceDestFile << ", ";
								}
								sourceDestFile << currTypeInfo.name;
							}
							sourceDestFile << ">" << endl;
						}
						sourceDestFile << returnType << " " << theInfo.classname << "::" << memberName << "()" << endl << "{" << endl;
						
						sourceDestFile << "}" << endl;
					} else if (currToken->type == semicolon) { // ivar
						
					} else if (currToken->type == equalsSign) { // ivar with default value
						skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
						go_next_token(tokens, currToken, fname);
						
						while (currToken->type != semicolon) {
							go_next_token(tokens, currToken, fname);
							skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
						}

						go_next_token(tokens, currToken, fname);
						skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
					} else {
						parse_error err(currToken->endOffset, currToken->endOffset, fname);
						err.message << "Expected \"(\", \"=\" or \";\", found \"" << currToken->text << "\".";
						throw err;
					}
				}
				
				headerDestFile << "};" << endl;

				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
				go_next_token(tokens, currToken, fname); // }
				skip_line_breaks(tokens, currToken, fname, headerDestFile, sourceDestFile);
			}
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
		const char*						suffix = ".vel";
		off_t suffixPos = sourceDestName.rfind(suffix);
		if (suffixPos != string::npos) {
			sourceDestName.erase(suffixPos, strlen(suffix));
		}
		string							headerDestName(sourceDestName);
		sourceDestName.append(".cpp");
		headerDestName.append(".hpp");

		cout << "note: output to " << sourceDestName << " and " << headerDestName << endl;

		ofstream						headerDestFile(headerDestName, ofstream::out | ofstream::trunc);
		ofstream						sourceDestFile(sourceDestName, ofstream::out | ofstream::trunc);
		headerDestFile << "/* This file was auto-generated using velocity from the file \"" << argv[1] << "\"." << endl
		<< "   Do not edit this file, all changes will be overwritten." << endl
		<< "   Edit the original file instead. */" << endl << endl
		<< "#pragma once" << endl << endl
		<< "#include \"object_class.hpp\"" << endl;

		sourceDestFile << "/* This file was auto-generated using velocity from the file \"" << argv[1] << "\"." << endl
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
			if( fname.rfind( ".vel" ) != fname.length() -4 )
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
