#pragma once

#include "scanner.h"
#include "defs.h"
#include "treeNode.h"

#include <memory>

class Scanner;

class Analyzer
{

  Scanner *m_Scanner;
  std::shared_ptr<treeNode> m_Root;
  std::shared_ptr<treeNode> m_Curr;


public:
  Analyzer(Scanner *scanner_)
    : m_Scanner{ scanner_ }
  {
    m_Root = treeNode::makeRoot();
    m_Curr = m_Root;
  }

  void addFunction(types, const std::string_view &);// input arg might be deleted out of scope (const char)
  void addArr(types, const std::string_view &, int, int);
  void addVar(types, const std::string_view &);
  void addScope();
  void exitScope();
  int getTypeOf(const std::string_view &, IdType);
  int getTypeOfFunc(); // Will go up until find func



  void printErr(const char *);
};