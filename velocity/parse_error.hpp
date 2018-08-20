//
//  parse_error.hpp
//  AgelessLang
//
//  Created by Uli Kusterer on 12.05.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#pragma once

#include <stdexcept>
#include <sstream>


using namespace std;


class parse_error : public runtime_error
{
public:
	explicit parse_error(size_t inStartOffset, size_t inEndOffset, const std::string& inFilename) : runtime_error(""), startOffset(inStartOffset), endOffset(inEndOffset), filename(inFilename) {}
	
	virtual const char* what() const throw()
	{
		string msg(filename);
		msg.append(":");
		msg.append(to_string(startOffset));
		msg.append(":");
		msg.append(to_string(endOffset));
		msg.append(":");
		msg.append(message.str());
		
		return msg.c_str();
	}

	stringstream message;
	
protected:
	size_t startOffset;
	size_t endOffset;
	string filename;
};
