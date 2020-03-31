//parser.cpp
#include "parser.h"
#include "defs.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <charconv>
#include "treeNode.h"

static bool isVarType(int type) { return (type == types::INT || type == types::BOOL); }


Parser::Parser(Scanner *sc)
{
  this->scanner = sc;
  this->m_Analyzer = new Analyzer(this->scanner);
  std::cout << "Running parser...\n";
  std::cout << "Running analyzer...\n";
}

void Parser::s()
{
  LOG("S");
  type = scanner->next();

  if (isVarType(type)) {
    funcOrVar(type);
    s();
  } else if (type == types::END) {
    return;
  } else {
    printErr((std::string("����������� �������: ") + std::string(this->scanner->getToken().data())).c_str());
  }
}


void Parser::funcOrVar(int type_)
{
  LOG("funcOrVar");

  int pos, col, row;// NOLINT
  scanner->getUk(pos, row, col);
  if (scanner->next() != types::ID) printErr("�������� �������������");
  const auto id = scanner->getToken();

  switch (scanner->next()) {
  case types::LBKT:
    scanner->setUk(pos, row, col);
    function(type_, id);
    break;
  case types::ASSIGN:
  case types::sLBKT:
  case types::SEMI:
  case types::COMA:
    scanner->setUk(pos, row, col);
    variables(type_);
    break;
  default:
    printErr("��������� ���������� ��� �������");
  }
}

void Parser::function(int type_, const std::string_view &id_)
{
  LOG("function");

  if (scanner->next() != types::ID) printErr("�������� �������������");
  if (scanner->next() != types::LBKT) printErr("��������� (");
  if (scanner->next() != types::RBKT) printErr("��������� )");

  m_Analyzer->addFunction((types)type_, id_);
  codeBlock();
}

void Parser::variables(int type_)
{
  LOG("variables");

  if (scanner->next() != types::ID) printErr("�������� �������������");

  const auto cachedToken = scanner->getToken();

  switch (scanner->next()) {
  case types::sLBKT: {
    if (scanner->next() != types::INT_CONST) printErr("��������� �������� ���������");
    const int size_fdim = std::stoi(scanner->getToken().data());
    if (scanner->next() != types::sRBKT) printErr("��������� ]");
    if (scanner->next() != types::sLBKT) printErr("��������� [");
    if (scanner->next() != types::INT_CONST) printErr("��������� �������� ���������");
    const int size_sdim = std::stoi(scanner->getToken().data());
    if (scanner->next() != types::sRBKT) printErr("��������� ]");

    m_Analyzer->addArr((types)type_, cachedToken, size_fdim, size_sdim);

    switch (scanner->next()) {
    case types::COMA:
      variables(type_);
      break;
    case types::SEMI:
    default:
      break;
    }
    break;
  }
  case types::ASSIGN:
    this->m_Analyzer->addVar((types)type_, cachedToken);
    if (type_ != expression()) this->m_Analyzer->printErr("�� ��������� ��� ���������� � ��� ���������");
    type = scanner->next();
    if (type == types::COMA) {
      variables(type_);
    } else if (type != types::SEMI) {
      printErr("��������� ;");
    }
    break;
  case types::SEMI:
    this->m_Analyzer->addVar((types)type_, cachedToken);
    break;
  case types::COMA:
    this->m_Analyzer->addVar((types)type_, cachedToken);
    variables(type_);
    break;
  default:
    this->printErr("������������ �������");
  }
}

void Parser::codeBlock()
{
  LOG("codeBlock");

  if (scanner->next() != types::fLBKT)
    printErr("��������� {");

  this->m_Analyzer->addScope();

  int pos, col, row;
  scanner->getUk(pos, row, col);
  type = scanner->next();

  while (type == types::INT || type == types::BOOL || type == types::IF || type == types::ID || type == types::fLBKT || type == types::RETURN) {
    if (type == types::INT || type == types::BOOL) {
      variables(type);
    } else if (type == types::IF) {
      if (scanner->next() != types::LBKT) printErr("��������� (");
      if (expression() != types::BOOL) this->m_Analyzer->printErr("�� ������� �������� � �������� ����");
      if (scanner->next() != types::RBKT) printErr("��������� )");
      codeBlock();
    } else if (type == types::ID) {
      const auto cachedId = this->scanner->getToken();

      switch (scanner->next()) {
      case types::LBKT:
        if (scanner->next() != types::RBKT) printErr("��������� )");
        this->m_Analyzer->getTypeOf(cachedId, IdType::tFunc);// checking for idType congruence. getType throw exception other way
        if (scanner->next() != types::SEMI) printErr("��������� ;");
        break;
      case types::ASSIGN:
        if (expression() != this->m_Analyzer->getTypeOf(cachedId, IdType::tVar)) this->m_Analyzer->printErr("�� ��������� ��� ���������� � ���������");
        if (scanner->next() != types::SEMI) printErr("��������� ;");
        break;
      case types::SUMEQ:
      case types::SUBEQ:
      case types::DIVEQ:
      case types::MULEQ:
        //scanner->setUk(pos, row, col);
        if (expression() != types::INT) this->m_Analyzer->printErr("������������ ��� ���������");
        if (scanner->next() != types::SEMI) printErr("��������� ;");
        break;
      case types::INC:
      case types::DEC:
        if (this->m_Analyzer->getTypeOf(cachedId, IdType::tVar) != types::INT) this->m_Analyzer->printErr("������������ ��� ���������");
        if (scanner->next() != types::SEMI) printErr("��������� ;");
        break;
      default:
        printErr("�������� ����� ������� ��� ������������");
        break;
      }
    } else if (type == types::fLBKT) {
      scanner->setUk(pos, row, col);
      codeBlock();
    } else if (type == types::RETURN) {
      if (expression() != this->m_Analyzer->getTypeOfFunc()) this->m_Analyzer->printErr("��� ��������� �� ��������� � ����� �������");
      if (scanner->next() != types::SEMI) printErr("��������� ;");
    }

    scanner->getUk(pos, row, col);
    type = scanner->next();
  }
  scanner->setUk(pos, row, col);

  if (scanner->next() != types::fRBKT) printErr("��������� }");
  this->m_Analyzer->exitScope();
}

int Parser::expression()
{
  LOG("expression");
  int pos, col, row;

  auto eltype = element();
  auto resType = eltype;

  scanner->getUk(pos, row, col);
  switch (scanner->next()) {
  case types::MORE:
  case types::LESS:
  case types::MEQ:
  case types::LEQ:
  case types::EQ:
    if (eltype != expression()) this->m_Analyzer->printErr("������ ���� ���������");
    resType = BOOL;
    break;

  default:
    scanner->setUk(pos, row, col);
    break;
  }
  return resType;
}

int Parser::element()
{
  LOG("element");
  int resType;

  // a = (a) | a
  int pos, col, row;
  scanner->getUk(pos, row, col);
  if (scanner->next() == types::LBKT) {
    resType = element();
    if (scanner->next() != types::RBKT) {
      printErr("��������� )");
    } else {
      goto checkOpperation;
    }
  } else {
    scanner->setUk(pos, row, col);
  }
  // -------

  int type2;
  switch (scanner->next()) {
  case types::ID: {
    const auto id = scanner->getToken();

    scanner->getUk(pos, row, col);
    type2 = scanner->next();
    if (type2 == types::sLBKT) {
      if (expression() != types::INT) this->m_Analyzer->printErr("�������� ��� ���������");
      if (scanner->next() != types::sRBKT) printErr("��������� ]");
      if (scanner->next() != types::sLBKT) printErr("��������� [");
      if (expression() != types::INT) this->m_Analyzer->printErr("�������� ��� ���������");
      if (scanner->next() != types::sRBKT) printErr("��������� ]");
      resType = this->m_Analyzer->getTypeOf(id, IdType::tArr);
    } else if (type2 == types::LBKT) {
      if (scanner->next() != types::RBKT) printErr("��������� )");
      resType = this->m_Analyzer->getTypeOf(id, IdType::tFunc);
    } else {
      resType = this->m_Analyzer->getTypeOf(id, IdType::tVar);
      scanner->setUk(pos, row, col);
    }

    break;
  }
  case types::BOOL_CONST:
    resType = BOOL;
    break;
  case types::INT_CONST:
    resType = INT;
    break;
  default:
    printErr("�������� �������");
    return -1;
  }

checkOpperation:
  scanner->getUk(pos, row, col);
  type = scanner->next();
  if (type == types::SUM || type == types::SUB || type == types::MUL || type == types::DIV || type == types::MULEQ || type == types::SUBEQ || type == types::SUMEQ || type == types::DIVEQ) {
    if (resType != element() || resType != types::INT) {
      this->m_Analyzer->printErr("������������ ���� ���������");
    } else if (type == types::INC || type == types::DEC) {
      if (resType != types::INT) this->m_Analyzer->printErr("������������ ��� ��������");
      return resType;
    }
  } else {
    scanner->setUk(pos, row, col);
  }


  return resType;
}

void Parser::printErr(const char *err)
{

  int pos, coll, row;
  this->scanner->getUk(pos, row, coll);
  std::cout << "[Parser] " << this->scanner->getFilename() << ":" << row + 1 << ":" << coll << " Error " << err << '\n';
  errors++;

  throw "";
  //return types::ERROR;
}
