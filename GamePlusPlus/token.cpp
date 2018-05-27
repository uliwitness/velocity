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


tokenType token::two_char_operator_for_types(tokenType firstChar, tokenType secondChar) {
	if (firstChar == lessThanOperator && secondChar == lessThanOperator) {
		return leftShiftOperator;
	} else if (firstChar == greaterThanOperator && secondChar == greaterThanOperator) {
		return rightShiftOperator;
	} else if (firstChar == plusOperator && secondChar == plusOperator) {
		return incrementOperator;
	} else if (firstChar == minusOperator && secondChar == minusOperator) {
		return decrementOperator;
	} else if (firstChar == equalsSign && secondChar == equalsSign) {
		return compareOperator;
	} else if (firstChar == plusOperator && secondChar == equalsSign) {
		return plusAssignmentOperator;
	} else if (firstChar == minusOperator && secondChar == equalsSign) {
		return minusAssignmentOperator;
	} else if (firstChar == multiplyOperator && secondChar == equalsSign) {
		return multiplyAssignmentOperator;
	} else if (firstChar == divideOperator && secondChar == equalsSign) {
		return divideAssignmentOperator;
	} else if (firstChar == moduloOperator && secondChar == equalsSign) {
		return moduloAssignmentOperator;
	} else if (firstChar == complementOperator && secondChar == equalsSign) {
		return complementAssignmentOperator;
	} else if (firstChar == orOperator && secondChar == equalsSign) {
		return orAssignmentOperator;
	} else if (firstChar == andOperator && secondChar == equalsSign) {
		return andAssignmentOperator;
	} else if (firstChar == tildeOperator && secondChar == equalsSign) {
		return tildeAssignmentOperator;
	} else if (firstChar == orOperator && secondChar == orOperator) {
		return logicalOrOperator;
	} else if (firstChar == andOperator && secondChar == andOperator) {
		return logicalAndOperator;
	} else if (firstChar == colonOperator && secondChar == colonOperator) {
		return scopeResolutionOperator;
	}
	return tokenType_LAST;
}


void token::end_token(token &newToken, vector<token> &tokens, tokenType currWhitespaceTokenType, size_t lineNumber)
{
	tokenType lastTokenType = tokens.empty() ? tokenType_LAST : tokens.back().type;
	
	newToken.lineNumber = lineNumber;
	if (newToken.startOffset < newToken.endOffset) {
		if (newToken.type == identifier) {
			if (strcasecmp(newToken.text.c_str(), "=") == 0) {
				newToken.type = equalsSign;
			} else if (strcasecmp(newToken.text.c_str(), ";") == 0) {
				newToken.type = semicolon;
			} else if (strcasecmp(newToken.text.c_str(), "{") == 0) {
				newToken.type = openCurlyBracket;
			} else if (strcasecmp(newToken.text.c_str(), "}") == 0) {
				newToken.type = closeCurlyBracket;
			} else if (strcasecmp(newToken.text.c_str(), "(") == 0) {
				newToken.type = openParenthesis;
			} else if (strcasecmp(newToken.text.c_str(), ")") == 0) {
				newToken.type = closeParenthesis;
			} else if (strcasecmp(newToken.text.c_str(), "[") == 0) {
				newToken.type = openSquareBracket;
			} else if (strcasecmp(newToken.text.c_str(), "]") == 0) {
				newToken.type = closeSquareBracket;
			} else if (strcasecmp(newToken.text.c_str(), ",") == 0) {
				newToken.type = commaOperator;
			} else if (strcasecmp(newToken.text.c_str(), ":") == 0) {
				newToken.type = colonOperator;
			} else if (strcasecmp(newToken.text.c_str(), "+") == 0) {
				newToken.type = plusOperator;
			} else if (strcasecmp(newToken.text.c_str(), "-") == 0) {
				newToken.type = minusOperator;
			} else if (strcasecmp(newToken.text.c_str(), "/") == 0) {
				newToken.type = divideOperator;
			} else if (strcasecmp(newToken.text.c_str(), "*") == 0) {
				newToken.type = multiplyOperator;
			} else if (strcasecmp(newToken.text.c_str(), "%") == 0) {
				newToken.type = moduloOperator;
			} else if (strcasecmp(newToken.text.c_str(), "^") == 0) {
				newToken.type = complementOperator;
			} else if (strcasecmp(newToken.text.c_str(), "|") == 0) {
				newToken.type = orOperator;
			} else if (strcasecmp(newToken.text.c_str(), "&") == 0) {
				newToken.type = andOperator;
			} else if (strcasecmp(newToken.text.c_str(), "~") == 0) {
				newToken.type = tildeOperator;
			} else if (strcasecmp(newToken.text.c_str(), ".") == 0) {
				newToken.type = dotOperator;
			} else if (strcasecmp(newToken.text.c_str(), "<") == 0) {
				newToken.type = lessThanOperator;
			} else if (strcasecmp(newToken.text.c_str(), ">") == 0) {
				newToken.type = greaterThanOperator;
			} else if (strcasecmp(newToken.text.c_str(), "?") == 0) {
				newToken.type = questionMarkOperator;
			} else if (strcasecmp(newToken.text.c_str(), "!") == 0) {
				newToken.type = exclamationMarkOperator;
			}
		} else if (newToken.type == lineBreak) {
			newToken.lineNumber = lineNumber - 1;
		}
		
		tokenType twoCharOperatorType = two_char_operator_for_types(lastTokenType, newToken.type);
		if (twoCharOperatorType != tokenType_LAST) {
			token &lastToken = tokens.back();
			lastToken.type = twoCharOperatorType;
			lastToken.text.append(newToken.text);
		} else {
			tokens.push_back(newToken);
		}
	}
	
	newToken.type = currWhitespaceTokenType;
	newToken.startOffset = SIZE_T_MAX;
	newToken.endOffset = SIZE_T_MAX;
	newToken.isInStringLiteralExpression = (currWhitespaceTokenType == stringLiteralExpression);
	newToken.text.erase();
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
