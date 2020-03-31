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

static constexpr DataType getType(types retType_)
{
  return retType_ == types::INT ? DataType::tInt : retType_ == types::BOOL ? DataType::tBool : throw "Cant recognize type";
}
static constexpr types getTypeInvert(DataType retType_)
{
  return retType_ == DataType::tInt ? types::INT : retType_ == DataType::tBool ? types::BOOL : throw "Cant recognize type";
}

void Analyzer::addFunction(types retType_, const std::string_view &id_)
{
  if (this->m_Curr->search(id_) != nullptr) this->printErr("Обьект с таким именем уже обьявлен");
  this->m_Curr = this->m_Curr->addId(id_, getType(retType_), IdType::tFunc);
}

void Analyzer::addArr(types retType_, const std::string_view &id_, int fdimsize_, int sdimsize_)
{
  if (this->m_Curr->search(id_) != nullptr) this->printErr("Обьект с таким именем уже обьявлен");
  this->m_Curr = this->m_Curr->addId(id_, getType(retType_), IdType::tArr, fdimsize_, sdimsize_);
}

void Analyzer::addVar(types retType_, const std::string_view &id_)
{
  LOG("AddVar");
  if (this->m_Curr->search(id_) != nullptr) this->printErr("Обьект с таким именем уже обьявлен");
  this->m_Curr = this->m_Curr->addId(id_, getType(retType_), IdType::tVar);
}

void Analyzer::addScope()
{
  this->m_Curr = this->m_Curr->addScope();
}

void Analyzer::exitScope()
{
  this->m_Curr = this->m_Curr->exitScope();
  if (this->m_Curr == nullptr) this->printErr("Не могу выйти из области");
}

int Analyzer::getTypeOf(const std::string_view &id_, IdType idtype_)
{
  auto res = this->m_Curr->search(id_);
  if (res == nullptr) printErr((std::string("Не найден обьект с именем ") + std::string(id_)).c_str());

  return getTypeInvert(res->getDataType());
}

int Analyzer::getTypeOfFunc()
{
  auto tmp = this->m_Curr->exitScope();
  if (tmp == nullptr) this->printErr("Не могу выйти из области");
  return getTypeInvert(tmp->getDataType());
}
