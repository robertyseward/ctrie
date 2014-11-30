#ifndef _CTRIE_H
#define _CTRIE_H
 
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
 
// TODO Consider getting rid of mParent.
// TODO Consider breaking STL conformance and adding a fast find() that
//      just returns a T* (no iterator).
// TODO Consider changing the way node strings are managed by having non-leaf
//      nodes have a fixed array of chars (maybe 16), where the first is the
//      string length, the second is the parent index, and the other 14 are
//      the string.  The leaf would remain a char* allocated separately.
// TODO Consider making a non-leaf node not have a value.  Instead, add a
//      leaf node to the beginning of mChildren that represents the null
//      character.
// TODO Add a regex iterator that would iterate over all entries that match
//      the regular expression.

namespace ctrie {

// Forward declarations
template<typename T, template<u_char> class Next, class Alloc> class _Leaf;
template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
    class _CmprNode;
template<typename T, u_char Sz, template<u_char> class Next, class Alloc>
    class _CmprValueNode;
template<typename T, template<u_char> class Next, class Alloc> class _FullNode;
template<typename T, template<u_char> class Next, class Alloc>
    class _FullValueNode;

} // end namespace ctrie

#include "ctrie_base.h"
#include "ctrie_leaf.h"
#include "ctrie_cmpr.h"
#include "ctrie_full.h"
#include "ctrie_main.h"

#endif
