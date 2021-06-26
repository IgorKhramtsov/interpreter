//analyzer.cpp
#include "analyzer.h"
#include <cstdio>
#include <iostream>
#include <string.h>


void Analyzer::printErr(const char *err)
{
  int pos, coll, row;
  this->m_Scanner->getUk(pos, row, coll);
  std::cout << "[Analyzer] " << this->m_Scanner->getFilename() << ":" << row + 1 << ":" << coll << " Error " << err << '\n';

  throw "";
  //return types::ERROR;
}

void Analyzer::show() {
  this->m_Root->print();
}

static constexpr DataType getType(types retType_)
{
  return retType_ == types::INT ? DataType::tInt : retType_ == types::BOOL ? DataType::tBool : throw "Cant recognize type";
}
static constexpr types getTypeInvert(DataType retType_)
{
  return retType_ == DataType::tInt ? types::INT : retType_ == DataType::tBool ? types::BOOL : throw "Cant recognize type";
}

Analyzer::Analyzer(Scanner *scanner_) : m_Scanner{ scanner_ }
{
  m_Root = treeNode::makeRoot();
  m_Curr = m_Root;
}


void Analyzer::addFunction(types retType_, const std::string_view &id_, Uk uk, std::map<std::string, int> args_)
{
  if (this->m_Curr->search(id_) != nullptr) this->printErr("Object with same name already defined");
  this->m_Curr = this->m_Curr->addFunc(id_, getType(retType_), uk, args_);
}

void Analyzer::addArr(types retType_, const std::string_view &id_, int fdimsize_, int sdimsize_)
{
  if (this->m_Curr->search(id_) != nullptr) this->printErr("Object with same name already defined");
  if (this->FlagInterp)
    this->m_Curr = this->m_Curr->getLeft();
  else
    this->m_Curr = this->m_Curr->addArr(id_, getType(retType_), fdimsize_, sdimsize_);
}

void Analyzer::addVar(types retType_, const std::string_view &id_, data_variant dat)
{
  LOG("AddVar");
  if (this->m_Curr->search(id_) != nullptr) this->printErr("Object with same name already defined");
  //if (this->m_Curr->getLeft()->getId() == id_)
    //this->m_Curr = this->m_Curr->getLeft();
  //else
    //this->m_Curr = this->m_Curr->addId(id_, getType(retType_), dat);
  if (!this->FlagInterp || this->FirstPass) {
    this->m_Curr = this->m_Curr->addId(id_, getType(retType_), dat);
  } else {
    this->m_Curr = this->m_Curr->getLeft();
    this->m_Curr->setVal(dat);
  }
}

void Analyzer::addScope()
{
  if (!FirstPass)
    this->m_Curr = this->m_Curr->getRight();
  else
    this->m_Curr = this->m_Curr->addScope();
}

void Analyzer::exitScope()
{
  this->m_Curr = this->m_Curr->exitScope();
  if (this->m_Curr == nullptr) this->printErr("Cant get out of scope");
}

int Analyzer::getTypeOf(const std::string_view &id_, IdType idtype_)
{
  treeNode *res;

  if (idtype_ == IdType::tFunc) {
    auto m1 = this->m_Curr->search(resolve_func(types::INT, id_));
    auto m2 = this->m_Curr->search(resolve_func(types::BOOL, id_));
    auto m3 = this->m_Curr->search(resolve_func(types::VOID, id_));

    res = m1 ? m1 : m2 ? m2 : m3;
  } else {
    res = this->m_Curr->search(id_);
  }
  if (res == nullptr) printErr((std::string("Cant find object with name ") + std::string(id_)).c_str());

  return getTypeInvert(res->getDataType());
}

// Look from root to all nodes
std::shared_ptr<treeNode> Analyzer::findById(const std::string_view &id_, bool preserveErr)
{
  auto res = this->m_Root->searchDown(id_);
  if (!preserveErr)
    if (res == nullptr) printErr((std::string("Cant find object with name ") + std::string(id_)).c_str());

  return res;
}

int Analyzer::getTypeOfFunc()
{
  auto tmp = getFuncNode();

  return getTypeInvert(tmp->getDataType());
}

void Analyzer::setFuncRet(data_variant ret_val)
{
  auto tmp = getFuncNode();
  tmp->setVal(ret_val);
}

std::shared_ptr<treeNode> Analyzer::getFuncNode()
{
  std::shared_ptr<treeNode> tmp = this->m_Curr;
  do {
    tmp = tmp->exitScope();
  } while (tmp->getIdType() != IdType::tFunc);
  if (tmp == nullptr) this->printErr("Cant get out of scope");

  return tmp;
}

void Analyzer::setVarVal(const std::string_view &id_, data_variant val_)
{
  auto res = this->m_Curr->search(id_);
  if (res == nullptr) printErr((std::string("Cant find object with name ") + std::string(id_)).c_str());
  auto type = res->getDataType();

  if (type == DataType::tBool) {
    if (!std::holds_alternative<bool>(val_)) printErr("Assigned value is not subtype of var");
  } else {
    if (!std::holds_alternative<int>(val_)) printErr("Assigned value is not subtype of var");
  }

  if (this->FlagInterp) res->setVal(val_);
}

data_variant Analyzer::getVarVal(const std::string_view &id_)
{
  auto res = this->m_Curr->search(id_);
  if (res == nullptr) printErr((std::string("Cant find object with name ") + std::string(id_)).c_str());

  return res->getVal();
}

data_variant Analyzer::getArrVal(const std::string_view &id_, int fdim, int sdim)
{
  auto res = this->m_Curr->search(id_);
  if (res == nullptr) printErr((std::string("Cant find object with name ") + std::string(id_)).c_str());

  return std::get<int **>(res->getVal())[fdim][sdim];
}

void Analyzer::setArrVal(const std::string_view &id_, int fdim, int sdim, data_variant dat)
{
  auto res = this->m_Curr->search(id_);
  if (res == nullptr) printErr((std::string("Cant find object with name ") + std::string(id_)).c_str());

  if (res->getDataType() == DataType::tInt)
    std::get<int **>(res->getVal())[fdim][sdim] = std::get<int>(dat);
  else
    std::get<int **>(res->getVal())[fdim][sdim] = std::get<bool>(dat);
}