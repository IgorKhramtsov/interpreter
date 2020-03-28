#pragma once
#include "defs.h"

#include <string>
#include <memory>

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

class treeNode
{
  std::string m_Id;
  IdType m_IdType;
  DataType m_DataType;
  int fdimsize;
  int sdimsize;


  std::shared_ptr<treeNode> m_LeftNode;// vars/funcs
  std::shared_ptr<treeNode> m_RigthNode;// scopes
  std::shared_ptr<treeNode> m_Parent;

public:
  treeNode(treeNode *_par, const std::string_view &id_ = "", const DataType dtype_ = DataType::none, const IdType itype_ = IdType::none, int fdimsize_ = 0, int sdimsize_ = 0) noexcept
    : m_Parent{ _par }, m_Id{ id_ }, m_IdType{ itype_ }, m_DataType{ dtype_ }, fdimsize{ fdimsize_ }, sdimsize{ sdimsize_ }
  {}

  std::shared_ptr<treeNode> addId(const std::string_view &, DataType, IdType, int = 0, int = 0);
  std::shared_ptr<treeNode> addScope();
  std::shared_ptr<treeNode> exitScope();
  std::shared_ptr<treeNode> getParent();
  treeNode *search(const std::string_view &);
  DataType getDataType() { return this->m_DataType; }

  static const std::shared_ptr<treeNode> makeRoot()
  {
    return std::make_shared<treeNode>(nullptr);
  }
};
