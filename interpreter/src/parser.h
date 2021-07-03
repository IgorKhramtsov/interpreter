#pragma once

#include "scanner.h"
#include <stack>
#include <map>
#include <vector>


struct Data
{
  int type;
  data_variant payload;
  Data(int tp_, data_variant pl_) : type{ tp_ }, payload{ pl_ } {}
};


class Parser
{
  Scanner *scanner;
  Analyzer *m_Analyzer;

  // interpreter
  bool FlagInterp = false;      // #1
  bool FirstPass = true;

  std::stack<Uk> callStack;
  std::map<std::string, Uk> funcs;
  
public:
  explicit Parser(Scanner *);
  ~Parser() = default;

  int errors = 0;

  void s();
  void startInterp();
  void show();
  data_variant callFunc(std::shared_ptr<treeNode>, std::vector<data_variant>);
  data_variant callFunc(const std::string_view &, std::vector<data_variant> = {});
  void funcOrVar(int);
  void function(int, const std::string_view &);
  std::map<std::string, int> params();
  std::vector<data_variant> arguments();
  void variables(int);
  void codeBlock(std::shared_ptr<treeNode> node_ = nullptr, std::vector<data_variant> args_ = {});
  Data expression();
  Data element();

  void setInterp(bool fl) {
    this->FlagInterp = fl;
    this->m_Analyzer->FlagInterp = fl;
  }
  void setFirstPass(bool fl) {
    this->FirstPass = fl;
    this->m_Analyzer->FirstPass = fl;
  }

  void printErr(const char*);
};

