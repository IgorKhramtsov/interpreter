#pragma once

#include "scanner.h"

class Parser
{
  Scanner *scanner;
  Analyzer *m_Analyzer;
  int type;
  
public:
  explicit Parser(Scanner *);
  ~Parser() = default;

  int errors = 0;

  void s();
  void funcOrVar(int);
  void function(int, const char* id_ );
  void variables(int);
  void codeBlock();
  int expression();
  int element();

  void printErr(const char*);
};

