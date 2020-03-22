#pragma once

#include "scanner.h"

class Parser
{
	Scanner *scanner;
	int type;
	
	//int pos, col, row;

public:
	explicit Parser(Scanner *);
	~Parser() = default;

	int errors = 0;

	void s();
	void funcOrVar();
	void function();
	void variables();
	void codeBlock();
	void expression();
	void element();

	void printErr(const char*);
};

