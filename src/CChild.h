#ifndef CCHILD_H
#define CCHILD_H

template<typename PARENT>
class CChild {
 protected:
  PARENT *parent_;

 public:
  CChild() : parent_(0) { }

  PARENT *getParent() const { return parent_; }

  void setParent(PARENT *parent) {
    parent_ = parent;
  }
};

#endif
