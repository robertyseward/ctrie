#ifndef _CTRIE_MAIN_H
#define _CTRIE_MAIN_H
 
namespace ctrie {

template<u_char Sz> struct Small {};
template<> struct Small<2> {
  static const u_char up = 4;
  static const u_char down = std::numeric_limits<u_char>::max();
  static const u_char downThreshold = 0;
};
template<> struct Small<4> {
  static const u_char up = 8;
  static const u_char down = 2;
  static const u_char downThreshold = 1;
};
template<> struct Small<8> {
  static const u_char up = 16;
  static const u_char down = 4;
  static const u_char downThreshold = 2;
};
template<> struct Small<16> {
  static const u_char up = 32;
  static const u_char down = 8;
  static const u_char downThreshold = 4;
};
template<> struct Small<32> {
  static const u_char up = std::numeric_limits<u_char>::max();
  static const u_char down = 16;
  static const u_char downThreshold = 8;
};
template<> struct Small<std::numeric_limits<u_char>::max()> {
  static const u_char up = 2;       // The initial size of a new node
  static const u_char down = 32;
  static const u_char downThreshold = 16;
};

template<u_char Sz> struct Medium {};
template<> struct Medium<4> {
  static const u_char up = 16;
  static const u_char down = std::numeric_limits<u_char>::max();
  static const u_char downThreshold = 0;
};
template<> struct Medium<16> {
  static const u_char up = std::numeric_limits<u_char>::max();
  static const u_char down = 4;
  static const u_char downThreshold = 2;
};
template<> struct Medium<std::numeric_limits<u_char>::max()> {
  static const u_char up = 4;       // The initial size of a new node
  static const u_char down = 16;
  static const u_char downThreshold = 8;
};

template<u_char Sz> struct Fast {};
template<> struct Fast<8> {
  static const u_char up = std::numeric_limits<u_char>::max();
  static const u_char down = std::numeric_limits<u_char>::max();
  static const u_char downThreshold = 0;
};
template<> struct Fast<std::numeric_limits<u_char>::max()> {
  static const u_char up = 8;       // The initial size of a new node
  static const u_char down = 8;
  static const u_char downThreshold = 4;
};

template<typename T,
    template<u_char Sz> class Next = Medium,
    class Alloc = std::allocator<T> >
class CTrie {
private:
  typedef _BaseNode<T,Next,Alloc> NodeT;
  typedef _Leaf<T,Next,Alloc> LeafT;

private:
  NodeT* mTop;
  size_t mSize;
    
public:
  typedef std::string key_type;
  typedef T value_type;
  typedef size_t size_type;

  class const_iterator;
  class iterator {
  private:
    NodeT* mCurrentNode;
    size_t mCurrentIndex;
      
  public:
    typedef T value_type;
    typedef T& reference;
    typedef T* pointer;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

  public:
    iterator() : mCurrentNode(nullptr), mCurrentIndex(0) {}
    iterator(const iterator&) = default;
    ~iterator() = default;

    iterator& operator=(const iterator&) = default;
    bool operator==(const iterator& x) const;
    bool operator!=(const iterator& x) const           {return !operator==(x);}
    T& operator*()                                     {return *operator->();}
    const T& operator*() const                         {return *operator->();}
    T* operator->();
    const T* operator->() const;
    iterator& operator++();
    iterator& operator--();

    key_type key() const;
    bool at_end() const;

  private:
    // The default of the current index is a special case to make end() fast.
    iterator(NodeT* n, size_t i=NodeT::endIndex())
      : mCurrentNode(n), mCurrentIndex(i) {}
    iterator(NodeT* n, size_t i, bool after)
      : mCurrentNode(n), mCurrentIndex(i) {init(after);}

    void init(bool after);
    void findLeaf(NodeT* p);
    void moveUpOne();

    friend class CTrie::const_iterator;
    friend class CTrie;
  };

  class const_iterator {
  public:
    typedef T value_type;
    typedef const T& reference;
    typedef const T* pointer;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

  private:
    iterator mIter;
      
  public:
    const_iterator() : mIter() {}
    const_iterator(const iterator& x) : mIter(x) {}
    const_iterator(const const_iterator&) = default;
    ~const_iterator() = default;

    const_iterator& operator=(const const_iterator&) = default;
    bool operator==(const const_iterator& x) const {return mIter == x.mIter;}
    bool operator!=(const const_iterator& x) const {return !operator==(x);}
    const value_type& operator*()                  {return *operator->();}
    const value_type* operator->()                 {return mIter.operator->();}
    const_iterator& operator++()                   {++mIter; return *this;}
    const_iterator& operator--()                   {--mIter; return *this;}
    key_type key() const                           {return mIter.key();}

  protected:
    const_iterator(NodeT* node, size_t index=NodeT::endIndex())
      : mIter(node, index) {}
    const_iterator(NodeT* node, size_t index, bool after)
      : mIter(node, index, after) {}

    friend class CTrie;
  };

  class reverse_iterator: public std::reverse_iterator<iterator> {
  public:
    reverse_iterator() : std::reverse_iterator<iterator>() {}
    reverse_iterator(const iterator& x) : std::reverse_iterator<iterator>(x) {}
    key_type key() const      {iterator p = this->base(); --p; return p.key();}
  };

  class const_reverse_iterator: public std::reverse_iterator<const_iterator> {
  public:
    const_reverse_iterator() : std::reverse_iterator<const_iterator>() {}
    const_reverse_iterator(const const_iterator& x)
            : std::reverse_iterator<const_iterator>(x) {}
    key_type key() const {const_iterator p = this->base(); --p; return p.key();}
  };

  class const_prefix_iter;
  class prefix_iter : private iterator {
  public:
    typedef T value_type;
    typedef T& reference;
    typedef T* pointer;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

  private:
    key_type mSearchStr;
    size_t mSearchStrIndex;
      
  public:
    prefix_iter() : iterator(), mSearchStrIndex(0) {}
    prefix_iter(const prefix_iter&) = default;
    ~prefix_iter() = default;

    prefix_iter& operator=(const prefix_iter& x) = default;
    bool operator==(const prefix_iter& x) const;
    bool operator!=(const prefix_iter& x) const;
    T& operator*()                             {return *operator->();}
    const T& operator*() const                 {return *operator->();}
    T* operator->()                            {return iterator::operator->();}
    const T* operator->() const                {return iterator::operator->();}
    prefix_iter& operator++();
    prefix_iter& operator--();

    iterator base() const                      {return iterator(*this);}
    key_type key() const                       {return iterator::key();}
    bool at_end() const                        {return iterator::at_end();}

  private:
    prefix_iter(NodeT* node, const key_type& searchStr, int);
    // A special constructor for end()
    prefix_iter(NodeT* node, const key_type& searchStr)
      : iterator(node), mSearchStr(searchStr), mSearchStrIndex(0) {}

    bool nextPrefix();

    friend class CTrie::const_prefix_iter;
    friend class CTrie;
  };

  class const_prefix_iter {
  public:
    typedef T value_type;
    typedef const T& reference;
    typedef const T* pointer;
    typedef std::ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

  private:
    prefix_iter mIter;
      
  public:
    const_prefix_iter() : mIter() {}
    const_prefix_iter(const prefix_iter& x) : mIter(x.mIter) {}
    const_prefix_iter(const const_prefix_iter&) = default;
    ~const_prefix_iter() = default;

    const_prefix_iter& operator=(const const_prefix_iter&) = default;
    bool operator==(const const_prefix_iter& x) const {return mIter == x.mIter;}
    bool operator!=(const const_prefix_iter& x) const {return !operator==(x);}
    const value_type& operator*()                  {return *operator->();}
    const value_type* operator->()                 {return mIter.operator->();}
    const_prefix_iter& operator++()                {++mIter; return *this;}
    const_prefix_iter& operator--()                {--mIter; return *this;}

    key_type key() const                           {return mIter.key();}
    bool at_end() const                            {return mIter.at_end();}
    const_iterator base() const                    {return mIter.base();}


  private:
    const_prefix_iter(NodeT* node, const key_type& searchStr, int dummy)
      : mIter(node, searchStr, dummy) {}
    const_prefix_iter(NodeT* node, const key_type& searchStr)
      : mIter(node, searchStr) {}

    friend class CTrie;
  };

  class reverse_prefix_iter: public std::reverse_iterator<prefix_iter> {
  public:
    reverse_prefix_iter() : std::reverse_iterator<prefix_iter>() {}
    reverse_prefix_iter(const prefix_iter& x)
        : std::reverse_iterator<prefix_iter>(x) {}
    key_type key() const   {prefix_iter p = this->base(); --p; return p.key();}
  };

  class const_reverse_prefix_iter:
      public std::reverse_iterator<const_prefix_iter> {
  public:
    const_reverse_prefix_iter() : std::reverse_iterator<const_prefix_iter>() {}
    const_reverse_prefix_iter(const const_prefix_iter& x)
        : std::reverse_iterator<const_prefix_iter>(x) {}
    key_type key() const
                     {const_prefix_iter p = this->base(); --p; return p.key();}
  };

public:
  CTrie() : mTop(nullptr), mSize(0) {}
  CTrie(const CTrie& x);
  CTrie(CTrie&& x);
  CTrie& operator=(const CTrie& x);
  CTrie& operator=(CTrie&& x);
  ~CTrie()                          {if (mTop) mTop->destroy();}

  size_t size() const               {return mSize;}
  bool empty() const                {return mSize == 0;}
  void clear();
  void swap(CTrie& x)               {swap(mTop, x.mTop); swap(mSize, x.mSize);}
  T& operator[](const key_type& key) {return *insert(key,T()).first;}

  std::pair<iterator, bool>
      insert(const char* keyData, size_t keyLen, const T& value);
  std::pair<iterator, bool> insert(const key_type& key, const T& value);
  template<class InputIterator>
    void insert(InputIterator first, InputIterator last);

  size_t erase(const key_type& key)     {return erase(key.data(), key.size());}
  size_t erase(const char* keyData, size_t keyLen = key_type::npos);
  void erase(iterator& iter);
  void erase(iterator first, const iterator& second);

  iterator find(const key_type& key, bool matchPart=false);
  const_iterator find(const key_type& key, bool matchPart=false) const;
  iterator find(const char* keyData, size_t keyLen = key_type::npos,
      bool matchPart=false);
  const_iterator find(const char* keyData, size_t keyLen = key_type::npos,
      bool matchPart=false) const;
  size_t count(const key_type& key, bool matchPart=false) const;
  size_t count(const char* keyData, size_t keyLen = key_type::npos,
      bool matchPart=false) const;
  iterator lower_bound(const key_type& key);
  const_iterator lower_bound(const key_type& key) const;
  iterator lower_bound(const char* keyData, size_t keyLen = key_type::npos);
  const_iterator lower_bound(
      const char* keyData, size_t keyLen = key_type::npos) const;
  iterator upper_bound(const key_type& key, bool matchPart=false);
  const_iterator upper_bound(const key_type& key, bool matchPart=false) const;
  iterator upper_bound(const char* keyData, size_t keyLen = key_type::npos,
      bool matchPart=false);
  const_iterator upper_bound(const char* keyData,
      size_t keyLen = key_type::npos, bool matchPart=false) const;
  std::pair<iterator,iterator> equal_range(
      const key_type& key, bool matchPart=false);
  std::pair<const_iterator,const_iterator> equal_range(
      const key_type& key, bool matchPart=false) const;
  std::pair<iterator,iterator> equal_range(const char* keyData,
      size_t keyLen = key_type::npos, bool matchPart=false);
  std::pair<const_iterator,const_iterator> equal_range(const char* keyData,
      size_t keyLen = key_type::npos, bool matchPart=false) const;

  iterator begin()
  { return iterator(mTop, NodeT::valueIndex(), false); }

  const_iterator begin() const
  { return const_iterator(mTop, NodeT::valueIndex(), false); }

  iterator end()
  { return iterator(mTop); }

  const_iterator end() const
  { return const_iterator(mTop); }

  reverse_iterator rbegin()
  { return reverse_iterator(end()); }

  const_reverse_iterator rbegin() const
  { return const_reverse_iterator(end()); }

  reverse_iterator rend()
  { return reverse_iterator(begin()); }

  const_reverse_iterator rend() const
  { return const_reverse_iterator(begin()); }

  prefix_iter prefix_begin(const key_type& prefix)
  { return prefix_iter(mTop, prefix, 0); }

  const_prefix_iter prefix_begin(const key_type& prefix) const
  { return const_prefix_iter(mTop, prefix, 0); }

  prefix_iter prefix_end()
  { return prefix_iter(mTop, std::string()); }

  const_prefix_iter prefix_end() const
  { return const_prefix_iter(mTop, std::string()); }

  reverse_prefix_iter prefix_rbegin(const key_type& prefix)
  { return reverse_prefix_iter(prefix_iter(mTop, prefix)); }

  const_reverse_prefix_iter prefix_rbegin(const key_type& prefix) const
  { return const_reverse_prefix_iter(const_prefix_iter(mTop, prefix)); }

  reverse_prefix_iter prefix_rend(const key_type& prefix)
  { return reverse_prefix_iter(prefix_begin( prefix)); }

  const_reverse_prefix_iter prefix_rend(const key_type& prefix) const
  { return const_reverse_prefix_iter(prefix_begin( prefix)); }

private:
  void maybeFixParentTable(NodeT* node, NodeT* replacementNode);

  friend class iterator;
  friend class const_iterator;
};

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::key_type
CTrie<T,Next,Alloc>::iterator::key() const
{
  key_type key;
  if (mCurrentNode == nullptr) {
    return key;
  }

  const NodeT *n;
  for (n = mCurrentNode; n->parent() != nullptr; n = n->parent()) {
    key.insert(0, n->str(), n->strLen());
    key.insert(0, 1, n->parentIndex());
  }
  key.insert(0,  n->str(), n->strLen());
  if (mCurrentIndex != NodeT::valueIndex()) {
    key += mCurrentNode->key(mCurrentIndex);
    NodeT* leaf = mCurrentNode->getEntry(mCurrentIndex);
    key.append(leaf->str(), leaf->strLen());
  }
  return key;
}

template<typename T, template<u_char> class Next, class Alloc>
inline T*
CTrie<T,Next,Alloc>::iterator::operator->()
{
  if (mCurrentIndex == NodeT::valueIndex()) {
    return &(mCurrentNode->value());
  } else {
    return &(mCurrentNode->getEntry(mCurrentIndex)->value());
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline const T*
CTrie<T,Next,Alloc>::iterator::operator->() const
{
  if (mCurrentIndex == NodeT::valueIndex()) {
    return &(mCurrentNode->value());
  } else {
    return &(mCurrentNode->getEntry(mCurrentIndex)->value());
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline bool
CTrie<T,Next,Alloc>::iterator::operator==(const iterator& x) const
{
  return mCurrentNode == x.mCurrentNode && mCurrentIndex == x.mCurrentIndex;
}

template<typename T, template<u_char> class Next, class Alloc>
inline bool
CTrie<T,Next,Alloc>::iterator::at_end() const
{
  return mCurrentNode == nullptr ||
      (mCurrentIndex == NodeT::endIndex() && mCurrentNode->parent() == nullptr);
}

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::iterator&
CTrie<T,Next,Alloc>::iterator::operator++()
{
  mCurrentIndex = mCurrentNode->nextEntry(mCurrentIndex);
  while (mCurrentIndex == NodeT::endIndex()) {
    // Move up the tree until we find a node with something left to iterate
    // over.
    if (mCurrentNode->parent() == nullptr) {
      return *this;
    }
    moveUpOne();
    mCurrentIndex = mCurrentNode->nextEntry(mCurrentIndex);
  }

  NodeT* entry = mCurrentNode->getEntry(mCurrentIndex);
  if (dynamic_cast<LeafT*>(entry) == nullptr) {
    findLeaf(entry);
  }
  return *this;
}

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::iterator&
CTrie<T,Next,Alloc>::iterator::operator--()
{
  bool goUp = mCurrentIndex == NodeT::valueIndex();
  if (!goUp) {
    mCurrentIndex = mCurrentNode->prevEntry(mCurrentIndex);
    goUp = mCurrentIndex == NodeT::valueIndex() && !mCurrentNode->hasValue();
  }
  while (goUp) {
    if (mCurrentNode->parent() == nullptr) {
      return *this;       // The user iterated before the first node
    }
    moveUpOne();
    mCurrentIndex = mCurrentNode->prevEntry(mCurrentIndex);
    goUp = mCurrentIndex == NodeT::valueIndex() && !mCurrentNode->hasValue();
  }
  while (1) {
    if (mCurrentIndex == NodeT::valueIndex()) {
      assert(mCurrentNode->hasValue());
      return *this;
    }
    NodeT* entry = mCurrentNode->getEntry(mCurrentIndex);
    if (dynamic_cast<LeafT*>(entry) != nullptr) {
      return *this;
    }
    mCurrentNode = entry;
    mCurrentIndex = mCurrentNode->lastEntry();
  }
  return *this;
}

template<typename T, template<u_char> class Next, class Alloc>
void
CTrie<T,Next,Alloc>::iterator::init(bool after)
{
  if (mCurrentNode == 0) {
    mCurrentIndex = NodeT::endIndex();
    return;
  }
  if (after && mCurrentIndex != NodeT::endIndex())
    mCurrentIndex = mCurrentNode->nextEntry(mCurrentIndex);

  // If we are after the end of this node, move up until we get something.
  while (mCurrentIndex == NodeT::endIndex()) {
    if (mCurrentNode->parent() == nullptr) {
      return;
    }
    moveUpOne();
    mCurrentIndex = mCurrentNode->nextEntry(mCurrentIndex);
  }

  // If the index has a corresponding value, use that; otherwise, traverse
  // down until we find a value.
  if (mCurrentIndex != NodeT::valueIndex()) {
    NodeT* entry = mCurrentNode->getEntry(mCurrentIndex);
    if (dynamic_cast<LeafT*>(entry) == nullptr) {
      findLeaf(entry);
    }
  } else if (mCurrentNode->hasValue() || !mCurrentNode->empty()) {
    findLeaf(mCurrentNode);
  } else {
    // There are no values in this nodes, so this better be the top node
    // of an empty tree.
    assert(mCurrentNode->parent() == nullptr);
    mCurrentIndex = NodeT::endIndex();
  }
}

template<typename T, template<u_char> class Next, class Alloc>
void
CTrie<T,Next,Alloc>::iterator::findLeaf(NodeT* node)
{
  mCurrentNode = node;
  mCurrentIndex = NodeT::valueIndex();
  while (!mCurrentNode->hasValue()) {
    mCurrentIndex = mCurrentNode->firstEntry();
    assert(mCurrentIndex != NodeT::endIndex());
    NodeT* entry = mCurrentNode->getEntry(mCurrentIndex);
    if (dynamic_cast<LeafT*>(entry) != nullptr) {
      break;
    }

    mCurrentNode = entry;
    mCurrentIndex = NodeT::valueIndex();
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline void
CTrie<T,Next,Alloc>::iterator::moveUpOne()
{
  char parentIndex = mCurrentNode->parentIndex();
  mCurrentNode = mCurrentNode->parent();
  mCurrentIndex = mCurrentNode->findEntry(parentIndex).first;
}

template<typename T, template<u_char> class Next, class Alloc>
inline
CTrie<T,Next,Alloc>::prefix_iter::prefix_iter(
      NodeT* node, const key_type& searchStr, int)
  : iterator(node, NodeT::valueIndex()),
    mSearchStr(searchStr),
    mSearchStrIndex(0)
{
  if (!this->mCurrentNode->hasValue() && !nextPrefix()) {
    this->mCurrentIndex = NodeT::endIndex();
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline bool
CTrie<T,Next,Alloc>::prefix_iter::operator==(const prefix_iter& x) const
{
  return iterator::operator==(x);
}

template<typename T, template<u_char> class Next, class Alloc>
inline bool
CTrie<T,Next,Alloc>::prefix_iter::operator!=(const prefix_iter& x) const
{
  return iterator::operator!=(x);
}

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::prefix_iter&
CTrie<T,Next,Alloc>::prefix_iter::operator++()
{
  if (!nextPrefix()) {
    // We are done, so create the end() iterator by moving up to the top.
    while (this->mCurrentNode->parent() != nullptr) {
      this->mCurrentNode = this->mCurrentNode->parent();
    }
    this->mCurrentIndex = NodeT::endIndex();
  }
  return *this;
}

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::prefix_iter&
CTrie<T,Next,Alloc>::prefix_iter::operator--()
{
  if (this->mCurrentNode->parent() == nullptr &&
      this->mCurrentIndex == NodeT::endIndex()) {
    // We are going backwards from the end, so traverse to the last
    // element, if there is one.
    this->mCurrentIndex = NodeT::valueIndex();
    while (nextPrefix()) {}
    if (this->mCurrentNode->parent() == nullptr &&
        this->mCurrentIndex == NodeT::valueIndex() &&
        !this->mCurrentNode->hasValue()) {
      // There is nothing to find
      this->mCurrentIndex = NodeT::endIndex();
    }
  } else if (this->mCurrentNode->parent() == nullptr &&
      this->mCurrentIndex == NodeT::valueIndex()) {
    // We were at the value of the top node, so we are done iterating
    this->mCurrentIndex = NodeT::endIndex();
  } else {
    if (this->mCurrentIndex != NodeT::valueIndex()) {
      // We are currently at a leaf, so move to the node
      this->mCurrentIndex = NodeT::valueIndex();
    } else {
      // We are currently at a node that is not the top node,
      // so move up a node.
      this->mCurrentNode = this->mCurrentNode->parent();
    }
    while (!this->mCurrentNode->hasValue() &&
        this->mCurrentNode->parent() != nullptr) {
      // This node has no value, so keep moving up
      this->mCurrentNode = this->mCurrentNode->parent();
    }
    if (!this->mCurrentNode->hasValue() &&
        this->mCurrentNode->parent() == nullptr)
      this->mCurrentIndex = NodeT::endIndex();
  }
  return *this;
}

/*
 * Advance the iterator to the next node that has this prefix.  If there is
 * no next node, the iterator does not change.
 * @return true if a next node was found.
 */
template<typename T, template<u_char> class Next, class Alloc>
bool
CTrie<T,Next,Alloc>::prefix_iter::nextPrefix()
{
  if (this->mCurrentIndex != NodeT::valueIndex())
    return false;   // The last value found was a leaf, so there is no more

  size_t nodeStrLen = this->mCurrentNode->strLen();
  if (this->mCurrentNode->parent() == nullptr && nodeStrLen) {
    // This is the top node, so we are just getting started.  Make sure the
    // beginning of mSearchStr matches this node.
    assert(mSearchStrIndex == 0);
    if (nodeStrLen > mSearchStr.length())
      return false;

    int cmpResult = mSearchStr.compare(
          mSearchStrIndex, nodeStrLen, this->mCurrentNode->str(), nodeStrLen);
    if (cmpResult != 0) {
      return false;
    }
    mSearchStrIndex += nodeStrLen;
  }

  // Move down the tree until we find a match.  Bail if we run out of search
  // string or no part of the tree matches the search string.
  NodeT* node = this->mCurrentNode;
  while (1) {
    if (mSearchStrIndex >= mSearchStr.length())
      return false;

    std::pair<size_t, bool> findRtn =
        node->findEntry(mSearchStr[mSearchStrIndex++]);
    if (!findRtn.second)
      return false;

    NodeT* entry = node->getEntry(findRtn.first);
    size_t entryStrLen = entry->strLen();
    if (entryStrLen) {
      // Compare this node's string against the search string.
      if (entryStrLen > mSearchStr.length() - mSearchStrIndex)
        return false;
      int cmpResult = mSearchStr.compare(
          mSearchStrIndex, entryStrLen, entry->str(), entryStrLen);
      if (cmpResult != 0) {
        return false;
      }
      mSearchStrIndex += entryStrLen;
    }
    if (dynamic_cast<LeafT*>(entry) != nullptr) {
      this->mCurrentNode = node;
      this->mCurrentIndex = findRtn.first;
      return true;
    }

    if (entry->hasValue()) {
      this->mCurrentNode = entry;
      this->mCurrentIndex = NodeT::valueIndex();
      return true;
    }
    node = entry;
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline CTrie<T,Next,Alloc>::CTrie(const CTrie& x)
  : mTop(nullptr), mSize(x.mSize)
{
  if (x.mTop) {
    mTop = x.mTop->clone();
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline CTrie<T,Next,Alloc>::CTrie(CTrie&& x)
  : mTop(x.mTop), mSize(x.mSize)
{
  x.mTop = nullptr;
  x.mSize = 0;
}

template<typename T, template<u_char> class Next, class Alloc>
CTrie<T,Next,Alloc>&
CTrie<T,Next,Alloc>::operator=(const CTrie& x)
{
  if (mTop) {
    mTop->destroy();
    mTop = nullptr;
  }
  if (x.mTop) {
    mTop = x.mTop->clone();
  }
  mSize = x.mSize;
  return *this;
}
    
template<typename T, template<u_char> class Next, class Alloc>
CTrie<T,Next,Alloc>&
CTrie<T,Next,Alloc>::operator=(CTrie&& x)
{
  if (mTop) {
    mTop->destroy();
    mTop = nullptr;
    mSize = 0;
  }
  std::swap(mTop, x.mTop);
  std::swap(mSize, x.mSize);
  return *this;
}
    
template<typename T, template<u_char> class Next, class Alloc>
inline void
CTrie<T,Next,Alloc>::clear()
{
  if (mTop) {
    mTop->destroy();
    mTop = nullptr;
    mSize = 0;
  }
}

template<typename T, template<u_char> class Next, class Alloc>
std::pair<typename CTrie<T,Next,Alloc>::iterator, bool>
CTrie<T,Next,Alloc>::insert(
    const char* searchKey, size_t keyLen, const T& value)
{
  typename NodeT::InsertRtn rtn;
  if (mTop == nullptr) {
    mTop = NodeT::createNode(nullptr, searchKey, keyLen, 0, value);
    rtn = NodeT::InsertRtn(mTop, NodeT::valueIndex(), true);
  } else {
    rtn = NodeT::insert(&mTop, searchKey, keyLen, 0, value);
  }
  if (rtn.succeeded)
      ++this->mSize;
  iterator iter(rtn.node, rtn.index, false);
  return std::make_pair(iter, rtn.succeeded);
}

template<typename T, template<u_char> class Next, class Alloc>
std::pair<typename CTrie<T,Next,Alloc>::iterator, bool>
CTrie<T,Next,Alloc>::insert(const key_type& searchKey, const T& value)
{
  if (mTop == nullptr) {
    mTop = NodeT::createNode(
        nullptr, searchKey.data(), searchKey.length(), 0, value);
    this->mSize = 1;
    return std::make_pair(iterator(mTop, NodeT::valueIndex(), false), true);
  }

  typename NodeT::InsertRtn rtn =
      NodeT::insert(&mTop, searchKey.data(), searchKey.length(), 0, value);
  if (rtn.succeeded)
    ++this->mSize;
  return std::make_pair(iterator(rtn.node, rtn.index, false), rtn.succeeded);
}

template<typename T, template<u_char> class Next, class Alloc>
template<class InputIterator>
void
CTrie<T,Next,Alloc>::insert(InputIterator firsti, InputIterator lasti)
{
  while (firsti != lasti) {
    typename NodeT::InsertRtn rtn = NodeT::insert(&mTop,
        firsti->first.data(), firsti->first.length(), 0, firsti->second);
    if (rtn.succeeded) {
      ++mSize;
    }
    ++firsti;
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline size_t
CTrie<T,Next,Alloc>::erase(const char* keyData, size_t keyLen)
{
  iterator p = find(keyData, keyLen);
  if (p != end()) {
      erase(p);
      return 1;
  } else {
      return 0;
  }
}

template<typename T, template<u_char> class Next, class Alloc>
void
CTrie<T,Next,Alloc>::erase(iterator& iter)
{
  --mSize;
  NodeT *node = iter.mCurrentNode;
  if (iter.mCurrentIndex == NodeT::valueIndex()) {
    // Remove the value from this node by moving the node to a non-value node.
    NodeT* newNode = node->moveRemoveValue();
    node->destroy();
    if (newNode->parent() == nullptr) {
      mTop = newNode;
    } else {
      NodeT *parent = newNode->parent();
      size_t thisIndex = parent->findEntry(newNode->parentIndex()).first;
      *parent->getEntryPtr(thisIndex) = newNode;
    }
    iter.mCurrentNode = newNode;
    if (!newNode->empty()) {
      iter.findLeaf(newNode);
      return;
    }
  } else {
    // Remove a leaf.
    NodeT* replacementNode;
    node->getEntry(iter.mCurrentIndex)->destroy();
    iter.mCurrentIndex = node->eraseEntry(iter.mCurrentIndex, &replacementNode);
    maybeFixParentTable(node, replacementNode);
    iter.mCurrentNode = replacementNode;
  }

  // Recursively clean up nodes that are now empty
  bool convertToLeaf = false;
  node = iter.mCurrentNode;
  while (node->empty()) {
    if (node->parent() == nullptr) {
      // Nothing left in the tree.
      assert(node == mTop);
      node->destroy();
      node = mTop = nullptr;
      iter = end();
      return;
    }
    NodeT* parent = iter.mCurrentNode = node->parent();
    iter.mCurrentIndex = parent->findEntry(node->parentIndex()).first;
    assert(iter.mCurrentIndex != NodeT::endIndex());

    if (node->hasValue()) {
      // Convert this node to a leaf.
      LeafT *leaf =
          LeafT::create(node->str(), node->strLen(), node->valueToMove());
      *parent->getEntryPtr(iter.mCurrentIndex) = leaf;
      node->destroy();
      convertToLeaf = true;
      break;
    }

    NodeT *replacementParent;
    iter.mCurrentIndex =
        parent->eraseEntry(iter.mCurrentIndex, &replacementParent);
    maybeFixParentTable(parent, replacementParent);
    node->destroy();
    node = iter.mCurrentNode = replacementParent;
  }

  // Make the iterator point to the next entry in the CTrie.  The iterator may
  // point to a non-leaf node that doesn't have a value.  Fix things up by
  // moving back one entry and then incrementing the iterator, which finds a
  // node with a value.
  if (!convertToLeaf) {
    iter.mCurrentIndex = node->prevEntry(iter.mCurrentIndex);
  }
  ++iter;
}

template<typename T, template<u_char> class Next, class Alloc>
void
CTrie<T,Next,Alloc>::erase(iterator first, const iterator& last)
{
  while (first != last) {
    iterator p = first;
    ++first;
    erase(p);
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline typename CTrie<T,Next,Alloc>::iterator
CTrie<T,Next,Alloc>::find(const key_type& key, bool matchPart)
{
  return find(key.data(), key.length(), matchPart);
}

template<typename T, template<u_char> class Next, class Alloc>
inline typename CTrie<T,Next,Alloc>::const_iterator
CTrie<T,Next,Alloc>::find(const key_type& key, bool matchPart) const
{
  return find(key.data(), key.length(), matchPart);
}

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::iterator
CTrie<T,Next,Alloc>::find(const char* keyData, size_t keyLen, bool matchPart)
{
  if (mTop == nullptr) {
    return end();
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  if (result.cmpValue == 0) {
    return iterator(result.node, result.index);
  } else if (matchPart && result.cmpValue == 1) {
    return iterator(result.node, result.index, false);
  } else {
    return end();
  }
}

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::const_iterator
CTrie<T,Next,Alloc>::find(const char* keyData, size_t keyLen, bool matchPart)
    const
{
  if (mTop == nullptr) {
    return end();
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  if (result.cmpValue == 0) {
    return const_iterator(result.node, result.index);
  } else if (matchPart && result.cmpValue == 1) {
    return const_iterator(result.node, result.index, false);
  } else {
    return end();
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline size_t
CTrie<T,Next,Alloc>::count(const key_type& key, bool matchPart) const
{
  return count(key.data(), key.length(), matchPart);
}

template<typename T, template<u_char> class Next, class Alloc>
size_t
CTrie<T,Next,Alloc>::count(const char* keyData, size_t keyLen, bool matchPart)
    const
{
  if (mTop == nullptr) {
    return 0;
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  if (result.cmpValue == 0 || (matchPart && result.cmpValue == 1)) {
    return matchPart && result.index == NodeT::valueIndex() ?
        result.node->treeSize() : 1;
  } else {
    return 0;
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline typename CTrie<T,Next,Alloc>::iterator
CTrie<T,Next,Alloc>::lower_bound(const key_type& key)
{
  return lower_bound(key.data(), key.size());
}

template<typename T, template<u_char> class Next, class Alloc>
inline typename CTrie<T,Next,Alloc>::const_iterator
CTrie<T,Next,Alloc>::lower_bound(const key_type& key) const
{
  return lower_bound(key.data(), key.size());
}

template<typename T, template<u_char> class Next, class Alloc>
inline typename CTrie<T,Next,Alloc>::iterator
CTrie<T,Next,Alloc>::lower_bound(const char* keyData, size_t keyLen)
{
  if (mTop == nullptr) {
    return end();
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  return iterator(result.node, result.index, result.cmpValue < 0);
}

template<typename T, template<u_char> class Next, class Alloc>
inline typename CTrie<T,Next,Alloc>::const_iterator
CTrie<T,Next,Alloc>::lower_bound(const char* keyData,
	size_t keyLen) const
{
  if (mTop == nullptr) {
    return end();
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  return const_iterator(result.node, result.index, result.cmpValue < 0);
}

template<typename T, template<u_char> class Next, class Alloc>
inline typename CTrie<T,Next,Alloc>::iterator
CTrie<T,Next,Alloc>::upper_bound(const key_type& key, bool matchPart)
{
  return upper_bound(key.data(), key.size(), matchPart);
}

template<typename T, template<u_char> class Next, class Alloc>
inline typename CTrie<T,Next,Alloc>::const_iterator
CTrie<T,Next,Alloc>::upper_bound(const key_type& key, bool matchPart) const
{
  return upper_bound(key.data(), key.size(), matchPart);
}

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::iterator
CTrie<T,Next,Alloc>::upper_bound(
    const char* keyData, size_t keyLen, bool matchPart)
{
  if (mTop == nullptr) {
    return end();
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  if (result.index == NodeT::valueIndex() && matchPart &&
      (result.cmpValue == 0 || result.cmpValue == 1)) {
    // We matched the entry in the node (not in the table) and 
    // all children nodes are meant to be before the given key, so
    // return the node after 'result.node'.  Force the index to be the
    // last element of the table to make this happen.
    return iterator(result.node, result.node->lastEntry(), true);

  } else if (result.cmpValue <= 0 || (matchPart && result.cmpValue == 1)) {
    // Only 'result.node' matches the key, so return the next node
    // after 'result.node'
    return iterator(result.node, result.index, true);

  } else {
    return iterator(result.node, result.index, false);
  }
}

template<typename T, template<u_char> class Next, class Alloc>
typename CTrie<T,Next,Alloc>::const_iterator
CTrie<T,Next,Alloc>::upper_bound(
    const char* keyData, size_t keyLen, bool matchPart)
    const
{
  if (mTop == nullptr) {
    return end();
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  if (result.index == NodeT::valueIndex() && matchPart &&
      (result.cmpValue == 0 || result.cmpValue == 1)) {
    // We matched the entry in the node (not in the table) and 
    // all children nodes are meant to be before the given key , so
    // return the node after 'result.node'.  Force the index to be the
    // last element of the table to make this happen.
    return const_iterator(result.node, result.node->lastEntry(), true);

  } else if (result.cmpValue <= 0 || (matchPart && result.cmpValue == 1)) {
    // Only 'result.node' matches the key, so return the next node
    // after 'result.node'
    return const_iterator(result.node, result.index, true);

  } else {
    return const_iterator( result.node, result.index, false );
  }
}

template<typename T, template<u_char> class Next, class Alloc>
inline std::pair<typename CTrie<T,Next,Alloc>::iterator,
    typename CTrie<T,Next,Alloc>::iterator>
CTrie<T,Next,Alloc>::equal_range(const key_type& key, bool matchPart)
{
  return equal_range(key.data(), key.size(), matchPart);
}

template<typename T, template<u_char> class Next, class Alloc>
inline std::pair<typename CTrie<T,Next,Alloc>::const_iterator,
    typename CTrie<T,Next,Alloc>::const_iterator>
CTrie<T,Next,Alloc>::equal_range(const key_type& key, bool matchPart) const
{
  return equal_range(key.data(), key.size(), matchPart);
}

template<typename T, template<u_char> class Next, class Alloc>
std::pair<typename CTrie<T,Next,Alloc>::iterator,
    typename CTrie<T,Next,Alloc>::iterator>
CTrie<T,Next,Alloc>::equal_range(
    const char* keyData, size_t keyLen, bool matchPart)
{
  if (mTop == nullptr) {
    return std::make_pair(end(), end());
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  if (result.index == NodeT::valueIndex() && matchPart &&
      (result.cmpValue == 0 || result.cmpValue == 1)) {
    return std::make_pair(iterator(result.node, NodeT::valueIndex(), false),
        iterator(result.node, result.node->lastEntry(), true));
  } else if (result.cmpValue == 0 || (matchPart && result.cmpValue == 1)) {
    return std::make_pair(iterator(result.node, result.index, false),
            iterator(result.node, result.index, true));
  } else {
    iterator iter = iterator(result.node, result.index, result.cmpValue < 0);
    return std::make_pair(iter, iter);
  }
}

template<typename T, template<u_char> class Next, class Alloc>
std::pair<typename CTrie<T,Next,Alloc>::const_iterator,
    typename CTrie<T,Next,Alloc>::const_iterator>
CTrie<T,Next,Alloc>::equal_range(
    const char* keyData, size_t keyLen, bool matchPart) const
{
  if (mTop == nullptr) {
    return std::make_pair(end(), end());
  }
  if (keyLen == key_type::npos) {
    keyLen = strlen(keyData);
  }
  typename NodeT::FindRtn result = mTop->find(keyData, keyLen);
  if (result.index == NodeT::valueIndex() && matchPart &&
      (result.cmpValue == 0 || result.cmpValue == 1)) {
    return std::make_pair(
        const_iterator(result.node, NodeT::valueIndex(), false),
        const_iterator(result.node, result.node->lastEntry(), true));
  } else if (result.cmpValue == 0 || (matchPart && result.cmpValue == 1)) {
    return std::make_pair(
        const_iterator(result.node, result.index, false),
        const_iterator(result.node, result.index, true));
  } else {
    const_iterator iter =
          const_iterator(result.node, result.index, result.cmpValue < 0);
    return std::make_pair(iter, iter);
  }
}

template<typename T, template<u_char> class Next, class Alloc>
void
CTrie<T,Next,Alloc>::maybeFixParentTable(NodeT* node, NodeT* replacementNode)
{
  if (node == replacementNode) {
    return;
  }

  // Removing an entry caused the node to be replaced with a node with
  // a smaller table.
  NodeT* parent = replacementNode->parent();
  if (parent == nullptr) {
    mTop = replacementNode;
  } else {
    size_t index = parent->findEntry(replacementNode->parentIndex()).first;
    *parent->getEntryPtr(index) = replacementNode;
  }
  node->destroy();
}

} // end namespace ctrie
#endif
