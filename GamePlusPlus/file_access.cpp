//
//  file_access.cpp
//  AgelessLang
//
//  Created by Uli Kusterer on 12.05.18.
//  Copyright © 2018 Uli Kusterer. All rights reserved.
//

#include <cstdio>
#include "file_access.hpp"


string file_contents(string filePath)
{
	FILE *file = fopen(filePath.c_str(), "r");
	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	string fileContents(length, ' ');
	fread(&(fileContents[0]), 1, length, file);
	
	return fileContents;
}


