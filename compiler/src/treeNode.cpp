#include "treeNode.h"

treeNode *treeNode::addId(const char *id_, const DataType dtype_, const IdType itype_)
{
  if (this->m_LeftNode != nullptr) {
    throw "Cant add treeNode to right. leftNode already exist";
  }

  this->m_LeftNode = new treeNode(this, id_, dtype_, itype_);


  return this->m_LeftNode;
}

treeNode *treeNode::addScope()
{
  if (this->m_RigthNode != nullptr) {
    throw "Cant add treeNode to right. rightNode already exist";
  }

  this->m_RigthNode = new treeNode(this);


  return this->m_RigthNode;
}

treeNode *treeNode::search(const char *search_id)
{
  if (strcmp(this->m_Id.c_str(), search_id))
    return this;
  else if (m_Parent != nullptr)
    return this->m_Parent->search(search_id);
  else
    return nullptr;
}
