//
//  token.cpp
//  AgelessLang
//
//  Created by Uli Kusterer on 12.05.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#include "token.hpp"
#include <cstdio>
#include <iomanip>


#define X(n)	#n,
const char *tokenTypeName[tokenType_LAST] = {
	TOKEN_TYPES
};
#undef X


void token::debug_print(ostream& outStream) const
{
	const string lineBreakText("↩︎");
	const string &prettyText = (type == lineBreak) ? lineBreakText : text;
	outStream << " [line=" << lineNumber << " start=" << setw(4) << startOffset << " end=" << setw(4) << endOffset << " isInExpression=" << isInStringLiteralExpression << " " << setw(25) << tokenTypeName[type] << "] " << prettyText << endl;
}


void token::debug_print(const vector<token>& tokens, ostream& outStream)
{
	for (auto currToken : tokens) {
		currToken.debug_print(cout);
	}
}


void token::end_token(token &currToken, vector<token> &tokens, tokenType currWhitespaceTokenType, size_t lineNumber)
{
	currToken.lineNumber = lineNumber;
	if (currToken.startOffset < currToken.endOffset) {
		if (currToken.type == identifier) {
			if (strcasecmp(currToken.text.c_str(), "=") == 0) {
				currToken.type = equalsSign;
			} else if (strcasecmp(currToken.text.c_str(), ";") == 0) {
				currToken.type = semicolon;
			} else if (strcasecmp(currToken.text.c_str(), "{") == 0) {
				currToken.type = openCurlyBracket;
			} else if (strcasecmp(currToken.text.c_str(), "}") == 0) {
				currToken.type = closeCurlyBracket;
			} else if (strcasecmp(currToken.text.c_str(), "(") == 0) {
				currToken.type = openParenthesis;
			} else if (strcasecmp(currToken.text.c_str(), ")") == 0) {
				currToken.type = closeParenthesis;
			} else if (strcasecmp(currToken.text.c_str(), "[") == 0) {
				currToken.type = openSquareBracket;
			} else if (strcasecmp(currToken.text.c_str(), "]") == 0) {
				currToken.type = closeSquareBracket;
			} else if (strcasecmp(currToken.text.c_str(), ",") == 0) {
				currToken.type = commaOperator;
			} else if (strcasecmp(currToken.text.c_str(), ":") == 0) {
				currToken.type = colonOperator;
			} else if (strcasecmp(currToken.text.c_str(), "+") == 0) {
				currToken.type = plusOperator;
			} else if (strcasecmp(currToken.text.c_str(), "-") == 0) {
				currToken.type = minusOperator;
			} else if (strcasecmp(currToken.text.c_str(), "/") == 0) {
				currToken.type = divideOperator;
			} else if (strcasecmp(currToken.text.c_str(), "*") == 0) {
				currToken.type = multiplyOperator;
			} else if (strcasecmp(currToken.text.c_str(), "%") == 0) {
				currToken.type = moduloOperator;
			} else if (strcasecmp(currToken.text.c_str(), "^") == 0) {
				currToken.type = complementOperator;
			} else if (strcasecmp(currToken.text.c_str(), "|") == 0) {
				currToken.type = orOperator;
			} else if (strcasecmp(currToken.text.c_str(), "&") == 0) {
				currToken.type = andOperator;
			} else if (strcasecmp(currToken.text.c_str(), "~") == 0) {
				currToken.type = tildeOperator;
			} else if (strcasecmp(currToken.text.c_str(), ".") == 0) {
				currToken.type = dotOperator;
			} else if (strcasecmp(currToken.text.c_str(), "<") == 0) {
				currToken.type = lessThanOperator;
			} else if (strcasecmp(currToken.text.c_str(), ">") == 0) {
				currToken.type = greaterThanOperator;
			} else if (strcasecmp(currToken.text.c_str(), "?") == 0) {
				currToken.type = questionMarkOperator;
			} else if (strcasecmp(currToken.text.c_str(), "!") == 0) {
				currToken.type = exclamationMarkOperator;
			}
		} else if (currToken.type == lineBreak) {
			currToken.lineNumber = lineNumber - 1;
		}
		
		tokens.push_back(currToken);
	}
	
	currToken.type = currWhitespaceTokenType;
	currToken.startOffset = SIZE_T_MAX;
	currToken.endOffset = SIZE_T_MAX;
	currToken.isInStringLiteralExpression = (currWhitespaceTokenType == stringLiteralExpression);
	currToken.text.erase();
}


inline bool is_whitespace(int c)
{
	return c == ' ' || c == '\t';
}


inline bool is_valid_identifier_char(int c)
{
	return !is_whitespace(c) && c != '\r' && c != '\n' && c != '=' && c != ';' && c != '{' && c != '}'  && c != '('  && c != ')' && c != ',' && c != ':' && c != '[' && c != ']' && c != '+' && c != '-' && c != '*' && c != '/' && c != '#' && c != '$' && c != '%' && c != '^' && c != '&' && c != '\"' && c != '\\' && c != '|' && c != '\'' && c != '.' && c != '<' && c != '>' && c != '?' && c != '!' && c != '`' && c != '@';
}


vector<token> token::tokenize(const string& fileContents)
{
	vector<token> tokens;
	token currToken;
	tokenType currWhitespaceTokenType = whitespace;
	size_t currLineNumber = 1;
	
	for (size_t x = 0; x < fileContents.length(); ++x) {
		char currCharacter = fileContents[x];
		
		if (currCharacter == '\r' && currToken.type != stringLiteral) {
			end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			currToken.type = lineBreak;
			currToken.startOffset = x;
			currToken.endOffset = x +1;
			currToken.text.append(1, currCharacter);
			currToken.lineNumber = currLineNumber;
			++currLineNumber;
		} else if (currCharacter == '\n' && currToken.type != stringLiteral) {
			if (currToken.type != lineBreak || currToken.text.compare("\r") != 0) {
				end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			}
			if (currToken.type != lineBreak) {
				currToken.type = lineBreak;
				currToken.startOffset = x;
				currToken.lineNumber = currLineNumber;
				++currLineNumber;
			}
			currToken.endOffset = x +1;
			currToken.text.append(1, currCharacter);
			end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
		} else if (isnumber(currCharacter) && (currToken.type == whitespace || currToken.type == stringLiteralExpression)) {
			end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			currToken.type = number;
			currToken.startOffset = x;
			currToken.endOffset = x +1;
			currToken.text.append(1, currCharacter);
		} else if(!isnumber(currCharacter) && !is_whitespace(currCharacter) && currToken.type == number) {
			if (!is_valid_identifier_char(currCharacter)) { // Starting an operator? End the number.
				end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
				currToken.startOffset = x;
			}
			currToken.type = identifier;
			currToken.endOffset = x +1;
			currToken.text.append(1, currCharacter);
			if (!is_valid_identifier_char(currCharacter)) { // Operator. It's a one-character token.
				end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			}
		} else if(currCharacter == '\"' && currToken.type == stringLiteral) {
			currToken.endOffset = x +1;
			end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
		} else if(currCharacter == '\"' && currToken.type != stringLiteral) {
			end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			currToken.type = stringLiteral;
			currToken.startOffset = x;
			currToken.endOffset = x +1;
		} else if(currToken.type == stringLiteral && currCharacter == '{') {
			currWhitespaceTokenType = stringLiteralExpression;
			end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
		} else if(currWhitespaceTokenType == stringLiteralExpression && currCharacter == '}') {
			currWhitespaceTokenType = whitespace;
			end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			currToken.type = stringLiteral;
			currToken.startOffset = x + 1;
			currToken.endOffset = x + 1;
			currToken.isInStringLiteralExpression = true;
		} else if(currToken.type == stringLiteral) {
			currToken.endOffset = x +1;
			currToken.text.append(1, currCharacter);
		} else if(currToken.type == identifier && is_valid_identifier_char(currCharacter)) {
			currToken.endOffset = x +1;
			currToken.text.append(1, currCharacter);
		} else if(currToken.type == identifier && !is_valid_identifier_char(currCharacter)) {
			end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			if (!is_whitespace(currCharacter)) { // An operator right after an identifier or operator?
				currToken.type = identifier;
				currToken.startOffset = x;
				currToken.endOffset = x +1;
				currToken.text.append(1, currCharacter);
				end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			}
		} else if((currToken.type == whitespace || currToken.type == stringLiteralExpression) && !is_whitespace(currCharacter)) {
			currToken.type = identifier;
			currToken.startOffset = x;
			currToken.endOffset = x +1;
			currToken.text.append(1, currCharacter);
			if (!is_valid_identifier_char(currCharacter)) { // Operator.
				end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
			}
		}
	}
	end_token(currToken, tokens, currWhitespaceTokenType, currLineNumber);
	
	return tokens;
}
