#pragma once

#include "scanner.h"
#include "defs.h"

class Scanner;

class Analyzer
{

  Scanner *m_Scanner;


public:
  Analyzer(Scanner *scanner_)
    : m_Scanner{ scanner_ }
  {}

  void addFunction(types, const char *);// input arg might be deleted out of scope (const char)
  void addArr(types, const char *, int, int);
  void addVar(types, const char *);
  void addScope();
  void exitScope();
  int getTypeOf(const char *, IdType);
  int getTypeOfFunc(); // Will go up until find func



  void printErr(const char *);
};