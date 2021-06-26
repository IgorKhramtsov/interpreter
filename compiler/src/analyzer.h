#pragma once

#include "scanner.h"
#include "defs.h"
#include "treeNode.h"

#include <memory>
#include <variant>
#include <map>

class Scanner;
class treeNode;
struct Uk;
enum struct DataType;
enum struct IdType;

class Analyzer
{

  Scanner *m_Scanner;
  std::shared_ptr<treeNode> m_Root;
  std::shared_ptr<treeNode> m_Curr;

  


public:
  Analyzer(Scanner *);

  bool FlagInterp = false;
  bool FirstPass = true;

  void addFunction(types, const std::string_view &, Uk, std::map<std::string, int>);
  void addArr(types, const std::string_view &, int, int);
  void addVar(types, const std::string_view &, data_variant = -1);
  void addScope();
  void exitScope();
  void setCurr(std::shared_ptr<treeNode> node_) { this->m_Curr = node_; }
  std::shared_ptr<treeNode> getCurr() { return this->m_Curr; }
  void setVarVal(const std::string_view &, data_variant);
  data_variant getVarVal(const std::string_view &);
  data_variant getArrVal(const std::string_view &, int, int);
  void setArrVal(const std::string_view &, int, int, data_variant);
  int getTypeOf(const std::string_view &, IdType);
  std::shared_ptr<treeNode> findById(const std::string_view &, bool = false);
  int getTypeOfFunc();// Will go up until find func
  std::shared_ptr<treeNode> getFuncNode();
  void setFuncRet(data_variant);


  void show();
  void printErr(const char *);
};