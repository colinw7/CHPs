#ifndef CCOMPOSITE_H
#define CCOMPOSITE_H

#include <vector>

template< typename THIS, typename CHILD, typename CHILD_LIST = std::vector<CHILD *> >
class CComposite {
 public:
  typedef typename CHILD_LIST::const_iterator const_child_iterator;
  typedef typename CHILD_LIST::iterator       child_iterator;

 public:
  CComposite(THIS *th) :
   this_(th) {
  }

  void add_child(CHILD *child) {
    children_.push_back(child);

    child->setParent(this_);
  }

  const CHILD_LIST &children() const { return children_; }

  const_child_iterator child_begin() const { return children_.begin(); }
  const_child_iterator child_end  () const { return children_.end  (); }

  child_iterator child_begin() { return children_.begin(); }
  child_iterator child_end  () { return children_.end  (); }

  void remove_child(child_iterator i) { children_.erase(i); }

 protected:
  THIS*      this_;
  CHILD_LIST children_;
};

#endif
