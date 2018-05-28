//
//  token.hpp
//  AgelessLang
//
//  Created by Uli Kusterer on 12.05.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#pragma once

#include <vector>
#include <iostream>
#include <string>


using namespace std;


#define TOKEN_TYPES		X(whitespace) \
						X(identifier) \
						X(number) \
						X(stringLiteral) \
						X(equalsSign) \
						X(semicolon) \
						X(openCurlyBracket) \
						X(closeCurlyBracket) \
						X(openSquareBracket) \
						X(closeSquareBracket) \
						X(openParenthesis) \
						X(closeParenthesis) \
						X(colonOperator) \
						X(commaOperator) \
						X(plusOperator) \
						X(minusOperator) \
						X(divideOperator) \
						X(multiplyOperator) \
						X(moduloOperator) \
						X(complementOperator) \
						X(orOperator) \
						X(andOperator) \
						X(tildeOperator) \
						X(dotOperator) \
						X(lessThanOperator) \
						X(greaterThanOperator) \
						X(questionMarkOperator) \
						X(exclamationMarkOperator) \
						X(leftShiftOperator) \
						X(rightShiftOperator) \
						X(incrementOperator) \
						X(decrementOperator) \
						X(compareOperator) \
						X(plusAssignmentOperator) \
						X(minusAssignmentOperator) \
						X(multiplyAssignmentOperator) \
						X(divideAssignmentOperator) \
						X(moduloAssignmentOperator) \
						X(complementAssignmentOperator) \
						X(orAssignmentOperator) \
						X(andAssignmentOperator) \
						X(tildeAssignmentOperator) \
						X(scopeResolutionOperator) \
						X(logicalOrOperator) \
						X(logicalAndOperator) \
						X(leftShiftAssignmentOperator) \
						X(rightShiftAssignmentOperator) \
						X(stringLiteralExpression) \
						X(lineBreak)


#define X(n)	n,
typedef enum tokenType {
	TOKEN_TYPES
	tokenType_LAST
} tokenType;
#undef X

class token
{
public:
	tokenType type = whitespace;
	size_t startOffset = SIZE_T_MAX;
	size_t endOffset = SIZE_T_MAX;
	size_t lineNumber = 0;
	string text;
	bool isInStringLiteralExpression;
	
	token() {}
	token(tokenType inType, string inText) : type(inType), text(inText) {}
	
	inline bool is_identifier(const string inString) const
	{
		if (type != identifier) { return false; }
		return (strcasecmp(inString.c_str(), text.c_str()) == 0);
	}
	
	void debug_print(ostream& outStream) const;
	
	string text_for_code() const {
		if (type == stringLiteral) {
			string quoted("\"");
			quoted.append(text);
			quoted.append("\"");
			return quoted;
		} else {
			return text;
		}
	}

	static vector<token> tokenize(const string& fileContents);
	static void debug_print(const vector<token>& tokens, ostream& outStream);
	
private:
	static void end_token(token &currToken, vector<token> &tokens, tokenType currWhitespaceTokenType, size_t lineNumber);
	static tokenType two_char_operator_for_types(tokenType firstChar, tokenType secondChar);
};
