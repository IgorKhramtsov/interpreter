#pragma once
#include "defs.h"
#include "scanner.h"

#include <string>
#include <memory>
#include <variant>

enum struct DataType {
  none = 0,// for scope
  tInt,
  tBool
};

enum struct IdType {
  none = 0,// for scope
  tVar,
  tFunc,
  tArr
};

struct Uk;

class treeNode
{
  std::string m_Id;
  IdType m_IdType;
  DataType m_DataType;
  int fdimsize;
  int sdimsize;
  data_variant value;
  std::map<std::string, int> arg_map;

  std::unique_ptr<Uk> scope_uk;


  std::shared_ptr<treeNode> m_LeftNode;// vars/funcs
  std::shared_ptr<treeNode> m_RigthNode;// scopes
  std::shared_ptr<treeNode> m_Parent;

public:
  treeNode(treeNode *_par, const std::string_view &id_ = "", const DataType dtype_ = DataType::none, const IdType itype_ = IdType::none, int fdimsize_ = 0, int sdimsize_ = 0) noexcept
    : m_Parent{ _par }, m_Id{ id_ }, m_IdType{ itype_ }, m_DataType{ dtype_ }, fdimsize{ fdimsize_ }, sdimsize{ sdimsize_ }
  {}

  std::shared_ptr<treeNode> addArr(const std::string_view &, DataType, int = 0, int = 0);
  std::shared_ptr<treeNode> addId(const std::string_view &, DataType, data_variant);
  std::shared_ptr<treeNode> addFunc(const std::string_view &, DataType, Uk, std::map<std::string, int>);
  std::shared_ptr<treeNode> addScope();
  std::shared_ptr<treeNode> exitScope();
  std::shared_ptr<treeNode> getParent();
  std::shared_ptr<treeNode> getRight() { return this->m_RigthNode; }
  std::shared_ptr<treeNode> getLeft() { return this->m_LeftNode; }
  const std::string &getId() { return this->m_Id; }
  treeNode *search(const std::string_view &);
  std::shared_ptr<treeNode> searchDown(const std::string_view &);
  DataType getDataType() { return this->m_DataType; }
  IdType getIdType() { return this->m_IdType; }
  Uk *getUk() { return this->scope_uk.get(); }

public:
  const std::shared_ptr<treeNode> propogateArgs(const std::shared_ptr<treeNode> &to, std::vector<data_variant> args_);
  const std::shared_ptr<treeNode> propogateArgs(const std::shared_ptr<treeNode> &to);

  static const std::shared_ptr<treeNode> makeRoot() noexcept;


  void setVal(data_variant val) { this->value = val; }
  data_variant getVal() { return this->value; }

  void print(int = 0);
};
