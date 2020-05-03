//parser.cpp
#include "parser.h"
#include "defs.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <charconv>
#include <variant>
#include "treeNode.h"

static bool isVarType(int type) { return (type == types::INT || type == types::BOOL); }

Parser::Parser(Scanner *sc)
{
  this->scanner = sc;
  this->m_Analyzer = new Analyzer(this->scanner);
  std::cout << "Running parser...\n";
  std::cout << "Running analyzer...\n";

  this->setInterp(true);
}

void Parser::s()
{
  LOG("S");
  auto type = scanner->next();

  if (isVarType(type)) {
    funcOrVar(type);
    s();
  } else if (type == types::END) {
    return;
  } else {
    printErr((std::string("Неизвестная лексема: ") + std::string(this->scanner->getToken().data())).c_str());
  }
}

void Parser::startInterp()
{
  setInterp(true);
  setFirstPass(false);
  auto ret = callFunc("main");
  std::cout << "Program exited with code " << std::get<int>(ret) << '\n';
}

data_variant Parser::callFunc(const std::string_view &name)
{
  auto m1 = m_Analyzer->findById(resolve_func(types::INT, name), true);
  auto m2 = m_Analyzer->findById(resolve_func(types::BOOL, name), true);
  auto m3 = m_Analyzer->findById(resolve_func(types::VOID, name), true);
  auto node = m1 ? m1 : m2 ? m2 : m3;

  if (node == nullptr) {
    this->m_Analyzer->printErr(std::string("Could not find method with name ").append(name).c_str());
  }
  return callFunc(node);
}

data_variant Parser::callFunc(std::shared_ptr<treeNode> node)
{
  if (this->FlagInterp) {

    this->callStack.push(this->scanner->getUk());
    auto tmp = this->m_Analyzer->getCurr();
    this->m_Analyzer->setCurr(node);
    this->scanner->setUk(*node->getUk());
    this->codeBlock();
    this->m_Analyzer->setCurr(tmp);
    this->scanner->setUk(this->callStack.top());
    this->callStack.pop();

    return node->getVal();
  } else {
    return -1;
  }
}

void Parser::funcOrVar(int type_)
{
  LOG("funcOrVar");

  int pos, col, row;// NOLINT
  scanner->getUk(pos, row, col);
  if (scanner->next() != types::ID) printErr("Ожидался идентификатор");
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
    printErr("Ожидалась переменная или функция");
  }
}

void Parser::function(int type_, const std::string_view &id_)
{
  LOG("function");

  if (scanner->next() != types::ID) printErr("Ожидался идентификатор");
  if (scanner->next() != types::LBKT) printErr("Ожидалось (");
  // args
  if (scanner->next() != types::RBKT) printErr("Ожидалось )");

  m_Analyzer->addFunction((types)type_, resolve_func(type_, id_ /*args*/), this->scanner->getUk());

  auto tmp = this->FlagInterp;
  setInterp(false);
  codeBlock();
  setInterp(tmp);
}

void Parser::variables(int type_)
{
  LOG("variables");

  if (scanner->next() != types::ID) printErr("Ожидался идентификатор");

  const auto cachedToken = scanner->getToken();

  switch (scanner->next()) {
  case types::sLBKT: {
    if (scanner->next() != types::INT_CONST) printErr("Ожидалась числовая константа");
    const int size_fdim = std::stoi(scanner->getToken().data());
    if (scanner->next() != types::sRBKT) printErr("Ожидалось ]");
    if (scanner->next() != types::sLBKT) printErr("Ожидалось [");
    if (scanner->next() != types::INT_CONST) printErr("Ожидалась числовая константа");
    const int size_sdim = std::stoi(scanner->getToken().data());
    if (scanner->next() != types::sRBKT) printErr("Ожидалось ]");

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
  case types::ASSIGN: {

    auto exp_dat = expression();
    if (type_ != exp_dat.type) this->m_Analyzer->printErr("Не совпадает тип переменной и тип выражения");
    this->m_Analyzer->addVar((types)type_, cachedToken, exp_dat.payload);

    auto type = scanner->next();
    if (type == types::COMA) {
      variables(type_);
    } else if (type != types::SEMI) {
      printErr("Ожидалось ;");
    }
    break;
  }
  case types::SEMI:
    this->m_Analyzer->addVar((types)type_, cachedToken);
    break;
  case types::COMA:
    this->m_Analyzer->addVar((types)type_, cachedToken);
    variables(type_);
    break;
  default:
    this->printErr("Недопустимая лексема");
  }
}

void Parser::codeBlock()
{
  LOG("codeBlock");

  if (scanner->next() != types::fLBKT)
    printErr("Ожидалось {");

  this->m_Analyzer->addScope();

  int pos, col, row;
  scanner->getUk(pos, row, col);
  auto type = scanner->next();

  while (type == types::INT || type == types::BOOL || type == types::IF || type == types::ID || type == types::fLBKT || type == types::RETURN) {
    if (type == types::INT || type == types::BOOL) {
      variables(type);
    } else if (type == types::IF) {
      if (scanner->next() != types::LBKT) printErr("Ожидалось (");
      auto exp_dat = expression();
      if (exp_dat.type != types::BOOL) this->m_Analyzer->printErr("Не удается привести к булевому типу");
      if (scanner->next() != types::RBKT) printErr("Ожидалось )");

      auto locFI = this->FlagInterp && std::get<bool>(exp_dat.payload);
      auto tmp = this->FlagInterp;
      setInterp(locFI);
      codeBlock();
      setInterp(tmp);
    } else if (type == types::ID) {
      const auto cachedId = this->scanner->getToken();

      auto typeOp = scanner->next();
      switch (typeOp) {
      case types::LBKT:
        if (scanner->next() != types::RBKT) printErr("Ожидалось )");
        //this->m_Analyzer->getTypeOf(cachedId, IdType::tFunc); // checking for idType congruence. getType throw exception other way

        callFunc(cachedId);
        if (scanner->next() != types::SEMI) printErr("Ожидалось ;");
        break;
      case types::ASSIGN: {

        auto exp_dat = expression();
        if (FlagInterp) {
          this->m_Analyzer->setVarVal(cachedId, exp_dat.payload);
        } else {
          if (exp_dat.type != this->m_Analyzer->getTypeOf(cachedId, IdType::tVar)) this->m_Analyzer->printErr("Не совпадает тип переменной и выражения");
        }

        if (scanner->next() != types::SEMI) printErr("Ожидалось ;");
        break;
      }
      case types::SUMEQ:
      case types::SUBEQ:
      case types::DIVEQ:
      case types::MULEQ: {
        auto exp_dat = expression();
        if (exp_dat.type != types::INT) this->m_Analyzer->printErr("Недопустимый тип выражения");

        if (this->FlagInterp) {
          int curVal = std::get<int>(this->m_Analyzer->getVarVal(cachedId));
          int expVal = std::get<int>(exp_dat.payload);
          if (typeOp == types::SUMEQ) curVal += expVal;
          if (typeOp == types::SUBEQ) curVal -= expVal;
          if (typeOp == types::DIVEQ) curVal /= expVal;
          if (typeOp == types::MULEQ) curVal *= expVal;
          this->m_Analyzer->setVarVal(cachedId, curVal);
        }

        if (scanner->next() != types::SEMI) printErr("Ожидалось ;");
        break;
      }
      case types::INC:
      case types::DEC: {

        if (FlagInterp) {
          auto value = std::get<int>(this->m_Analyzer->getVarVal(cachedId));
          if (typeOp == types::INC) this->m_Analyzer->setVarVal(cachedId, value + 1);
          if (typeOp == types::DEC) this->m_Analyzer->setVarVal(cachedId, value - 1);
        } else {
          if (this->m_Analyzer->getTypeOf(cachedId, IdType::tVar) != types::INT) this->m_Analyzer->printErr("Недопустимый тип выражения");
        }

        if (scanner->next() != types::SEMI) printErr("Ожидалось ;");
        break;
      }
      default:
        printErr("Ожидался вызов функции или присваивание");
        break;
      }
    } else if (type == types::fLBKT) {
      scanner->setUk(pos, row, col);
      codeBlock();
    } else if (type == types::RETURN) {
      auto exp_dat = expression();
      if (exp_dat.type != this->m_Analyzer->getTypeOfFunc()) this->m_Analyzer->printErr("Тип выражения не совпадает с типом функции");
      if (FlagInterp) this->m_Analyzer->setFuncRet(exp_dat.payload);

      if (scanner->next() != types::SEMI) printErr("Ожидалось ;");

      if (this->FlagInterp) return;
    }

    scanner->getUk(pos, row, col);
    type = scanner->next();
  }
  scanner->setUk(pos, row, col);

  if (scanner->next() != types::fRBKT) printErr("Ожидалось }");
  this->m_Analyzer->exitScope();
}

Data Parser::expression()
{
  LOG("expression");
  int pos, col, row;

  auto left_el = element();
  auto resType = left_el.type;
  data_variant resVal;

  scanner->getUk(pos, row, col);
  auto type = scanner->next();
  switch (type) {
  case types::MORE:
  case types::LESS:
  case types::MEQ:
  case types::LEQ:
  case types::EQ: {

    auto right_el = expression();
    if (left_el.type != right_el.type) this->m_Analyzer->printErr("Разные типы выражений");
    resType = BOOL;

    if (FlagInterp) {
      switch (type) {
      case types::MORE:
        resVal = left_el.payload > right_el.payload;
        break;
      case types::LESS:
        resVal = left_el.payload < right_el.payload;
        break;
      case types::MEQ:
        resVal = left_el.payload >= right_el.payload;
        break;
      case types::LEQ:
        resVal = left_el.payload <= right_el.payload;
        break;
      case types::EQ:
        resVal = left_el.payload == right_el.payload;
        break;
      }
    }

    break;
  }

  default:
    scanner->setUk(pos, row, col);
    resVal = left_el.payload;
    break;
  }

  return Data(resType, resVal);
}

Data Parser::element()
{
  LOG("element");
  int resType;
  data_variant resVal;
  std::string left_name;

  // a = (a) | a
  int pos, col, row;
  scanner->getUk(pos, row, col);
  if (scanner->next() == types::LBKT) {
    auto el = element();
    resType = el.type;
    resVal = el.payload;
    if (scanner->next() != types::RBKT) printErr("Ожидалось )");
    goto checkOpperation;
  } else {
    scanner->setUk(pos, row, col);
  }
  // -------

  switch (scanner->next()) {
  case types::ID: {
    const auto cached_id = scanner->getToken();

    scanner->getUk(pos, row, col);
    auto type = scanner->next();
    if (type == types::sLBKT) {
      auto first_expr = expression();
      if (first_expr.type != types::INT) this->m_Analyzer->printErr("Неверный тип выражения");
      if (scanner->next() != types::sRBKT) printErr("Ожидалось ]");
      if (scanner->next() != types::sLBKT) printErr("Ожидалось [");
      auto second_expr = expression();
      if (second_expr.type != types::INT) this->m_Analyzer->printErr("Неверный тип выражения");
      if (scanner->next() != types::sRBKT) printErr("Ожидалось ]");
      resType = this->m_Analyzer->getTypeOf(cached_id, IdType::tArr);
      resVal = this->m_Analyzer->getArrVal(cached_id, std::get<int>(first_expr.payload), std::get<int>(second_expr.payload));
    } else if (type == types::LBKT) {
      if (scanner->next() != types::RBKT) printErr("Ожидалось )");
      resType = this->m_Analyzer->getTypeOf(cached_id, IdType::tFunc);
      resVal = callFunc(cached_id);
    } else {
      resType = this->m_Analyzer->getTypeOf(cached_id, IdType::tVar);
      resVal = this->m_Analyzer->getVarVal(cached_id);
      left_name = cached_id;
      scanner->setUk(pos, row, col);
    }

    break;
  }
  case types::BOOL_CONST:
    resType = BOOL;
    resVal = this->scanner->getToken().data() == "true" ? true : false;
    break;
  case types::INT_CONST:
    resType = INT;
    resVal = std::stoi(this->scanner->getToken().data());
    break;
  default:
    printErr("Ожидался элемент");
  }

checkOpperation:
  scanner->getUk(pos, row, col);
  auto type = scanner->next();
  if (type == types::SUM || type == types::SUB || type == types::MUL || type == types::DIV || type == types::MULEQ || type == types::SUBEQ || type == types::SUMEQ || type == types::DIVEQ) {
    auto right_el = element();
    auto left_el = Data(resType, resVal);
    if (resType != types::INT || right_el.type != types::INT) this->m_Analyzer->printErr("Недопустимые типы элементов (допустимы только целочисленные)");


    if (this->FlagInterp) {
      auto leftVal = std::get<int>(left_el.payload);
      auto rightVal = std::get<int>(right_el.payload);
      switch (type) {
      case types::SUM:
        resVal = leftVal + rightVal;
        break;
      case types::SUB:
        resVal = leftVal - rightVal;
        break;
      case types::MUL:
        resVal = leftVal * rightVal;
        break;
      case types::DIV:
        resVal = leftVal / rightVal;
        break;
      }
      if (!left_name.empty() && 
        (type == types::SUMEQ || type == types::SUBEQ || type == types::MULEQ || type == types::DIVEQ)) {
        switch (type) {
        case types::SUMEQ:
          leftVal = leftVal + rightVal;
          break;
        case types::SUBEQ:
          leftVal = leftVal - rightVal;
          break;
        case types::MULEQ:
          leftVal = leftVal * rightVal;
          break;
        case types::DIVEQ:
          leftVal = leftVal / rightVal;
          break;
        }
        this->m_Analyzer->setVarVal(left_name, leftVal);
        resVal = leftVal;
      }
    }

  } else if (type == types::INC || type == types::DEC) {
    if (!left_name.empty()) {
      auto resValInt = std::get<int>(resVal);
      if (type == types::INC) resValInt++;
      if (type == types::DEC) resValInt--;

      if (this->FlagInterp) this->m_Analyzer->setVarVal(left_name, resValInt);
      resVal = data_variant(resValInt);
    } else {
      this->m_Analyzer->printErr("Операция применима только к переменным");
    }

  } else {
    scanner->setUk(pos, row, col);
  }


  return Data(resType, resVal);
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
