#include "ctrie.h"

#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>

// TODO move to using gtest style of ASSERT and EXPECT.
// TODO Add a debug option instead of commenting out various pieces.
// TODO Add an option to specify the number of words to read.
using namespace std;
using namespace ctrie;

typedef CTrie<int,Medium> IntCTrie;

size_t
uintRand(size_t interval)
{
  return static_cast<size_t>(
      static_cast<double>(interval) * rand() / (RAND_MAX + 1.0));
}

/*
 * Find all prefixes of {@code str} that are in the given map.
 */
vector<string>
getExpectedPrefixes(const string& str, const map<string,int>& map)
{
  vector<string> expected;
  for (size_t len = 0; len <= str.length(); ++len) {
    string substr = str.substr(0, len);
    if (map.find(substr) != map.end()) {
      expected.push_back(substr);
    }
  }
  return expected;
}

void checkConst(const IntCTrie& cmap);
int main()
{
  IntCTrie cmap;
  map<string,int> refMap;
  string word;
  int wordCount = 0;

  // Read in all the words and populate the test map and reference map.
  // We make the value the word position in the input file.
  while (cin >> word) {
//    cout << "Adding '" << word << "' (#" << wordCount << ")\n";
    pair<IntCTrie::iterator, bool> rtn = cmap.insert(word, wordCount);
    refMap.insert(make_pair(word, wordCount));
    if (!rtn.second) {
      cout << "ERROR: Return value for insert of " << word <<
	  " indicates failure" << endl;
    }
    if (rtn.first.key() != word) {
      cout << "ERROR: Key of return iterator for insert of " << word <<
	  " is wrong: " << rtn.first.key() << endl;
    }
    if (*rtn.first != wordCount) {
      cout << "ERROR: Value of return iterator for insert of " << word <<
	  " should be " << wordCount << " and is " << *rtn.first << endl;
    }
    if (cmap.size() != static_cast<size_t>(wordCount + 1)) {
      cout << "ERROR: Size of CTrie map should be " << (wordCount+1) <<
	  " after insert of " << word << " and is " << cmap.size() << endl;
    }
    ++wordCount;
  }
  cout << "A total of " << wordCount << " words were entered\n";

  if (cmap.size() != refMap.size()) {
      cout << "ERROR: Sizes differ: ref map size = " << refMap.size() << ", " <<
	      "CTrie map size = " << cmap.size() << endl;
  }

  // Iterate through the test and reference map, making sure the keys and
  // values are the same.
  cout << "Checking that all entries are in the map\n";
  map<string,int>::iterator rp = refMap.begin();
  IntCTrie::iterator cp = cmap.begin();
  wordCount = 0;
  for (; rp != refMap.end() && cp != cmap.end(); ++rp, ++cp, ++wordCount) {
//    cout << "Comparing '" << rp->first << "' (#" << rp->second << ")\n";
    if (rp->first != cp.key()) {
      cout << "ERROR: Keys differ on entry " << wordCount << ": " <<
	  "ref key = '" << rp->first << "', " <<
	  "CTrie key = '" << cp.key() << "'" << endl;
    }
    if (rp->second != *cp) {
      cout << "ERROR: Values differ on entry " << wordCount <<
	  "(key=" << rp->first << "): ref value = '" << rp->second << "', " <<
	  "CTrie value = '" << *cp << "'" << endl;
    }
  }
  if (rp != refMap.end()) {
    cout << "ERROR: Got to end of CTrie but not to the end of the ref map" << 
	endl;
  }
  if (cp != cmap.end()) {
    cout << "ERROR: Got to end of the ref map " <<
	"but not to the end of the CTrie" << endl;
  }

  // Reverse Iterate through the test and reference map, making sure the keys
  // are the same.
  cout << "Reverse iterating through the map\n";
  map<string,int>::reverse_iterator reverseRp;
  IntCTrie::reverse_iterator reverseCp;
  wordCount = refMap.size();
  for (reverseRp = refMap.rbegin(), reverseCp = cmap.rbegin();
      reverseRp != refMap.rend() && reverseCp != cmap.rend();
      ++reverseRp, ++reverseCp) {
//    cout << "Comparing '" << rp->first << "' (#" << rp->second << ")\n";
    if (reverseRp->first != reverseCp.key()) {
      cout << "ERROR: Keys differ on entry " << wordCount << ": " <<
	  "ref key = '" << reverseRp->first << "', " <<
	  "CTrie key = '" << reverseCp.key() << "'" << endl;
    }
    --wordCount;
  }
  if (reverseRp != refMap.rend()) {
    cout << "ERROR: Got to beginning of CTrie " <<
	"but not to the beginning of the ref map" << endl;
  }
  if (reverseCp != cmap.rend()) {
    cout << "ERROR: Got to beginning of the ref map " <<
	"but not to the beginning of the CTrie" << endl;
  }

  // Now go through every key and do a find(), lower_bound(), upper_bound(),
  // and equal_range() on both maps.  The results should be the same.
  cout << "Checking find(), lower_bound(), upper_bound(), and equal_range()\n";
  IntCTrie::iterator lower, upper;
  pair<IntCTrie::iterator, IntCTrie::iterator> range;
  for (rp = refMap.begin(); rp != refMap.end(); ++rp) {
//    cout << "Looking for '" << rp->first << "\n";
    cp = cmap.find(rp->first);
    if (rp->first != cp.key()) {
      cout << "ERROR: find() failed: ref key = '" << rp->first << "', " <<
	  "'CTrie key = '" << cp.key() << "'" << endl;
    }

    if (cmap.count(rp->first) != 1) {
      cout << "ERROR: count() failed: ref key = '" << rp->first << "'" << endl;
    }

    lower = cmap.lower_bound(rp->first);
    if (rp->first != lower.key()) {
      cout << "ERROR: lower_bound() failed: ref key = '" << rp->first << "'," <<
	  "CTrie key = '" << lower.key() << "'" << endl;
    }

    // Note that upper bound gives us the entry AFTER this key, so it
    // can be after the end of the map.
    upper = cmap.upper_bound(rp->first);
    ++rp;
    if (rp == refMap.end() && upper == cmap.end()) {
	;          // Ok - both are at end
    } else if (rp == refMap.end() && upper != cmap.end()) {
      cout << "ERROR: upper_bound failed: ref key at end, " <<
	  "CTrie key = '" << upper.key() << "'" << endl;
    } else if (rp != refMap.end() && upper == cmap.end()) {
      cout << "ERROR: upper_bound failed: ref key = '" << rp->first <<
	  "', CTrie key at end" << endl;
    } else if (rp->first != upper.key()) {
      cout << "ERROR: upper_bound failed: ref key = '" << rp->first <<
	  "', CTrie key = '" << upper.key() << "'" << endl;
    }
    --rp;

    range = cmap.equal_range(rp->first);
    if (range.first != lower || range.second != upper) {
      cout << "ERROR: equal_range failed: " <<
	  "key = '" << rp->first << "'" << endl;
    }
  }

  // Now test the match partial feature.  We do this by getting every
  // substring in all keys, and finding all entries that match that
  // substring.
  cout << "Checking the match partial feature\n";
  map<string,int>::iterator refLower;
  bool hasEntries = true;

  for (size_t len = 1; hasEntries; ++len) {
    hasEntries = false;
    rp = refMap.begin();
    while (1) {
      while (rp != refMap.end() && rp->first.length() < len) {
	++rp;
      }
      if (rp == refMap.end()) {
	break;
      }

      hasEntries = true;
      refLower = rp;
      string substr(rp->first, 0, len);
      size_t numEntries = 0;

      while (rp != refMap.end() &&
	  rp->first.length() >= len &&
	  strncmp(substr.data(), rp->first.data(), len) == 0) {
	++numEntries;
	++rp; 
      }
//      cout << "Substring '" << substr << "' has " << numEntries << " entries\n";

      size_t cmap_entries = cmap.count(substr, true);
      if (numEntries != cmap_entries) {
	cout << "ERROR: count with match part failed: " <<
	    "substring = '" << substr << "', " <<
	    "ref # entries = " << numEntries << ", " <<
	    "CTrie # entries = " << cmap_entries << "'" << endl;
      }

      cp = cmap.find(substr, true);
      if (refLower->first != cp.key()) {
	cout << "ERROR: find() with match part failed: " <<
	    "substring = '" << substr << "', " <<
	    "ref key = '" << refLower->first << "', " <<
	    "CTrie key = '" << cp.key() << "'" << endl;
      }

      cp = cmap.upper_bound(substr, true);
      if (rp == refMap.end() && cp == cmap.end()) {
	;          // Ok - both are at end
      } else if (rp == refMap.end() && cp != cmap.end()) {
	cout << "ERROR: upper_bound with match part failed: " <<
	    "substring = '" << substr << "', " << "ref key at end, " <<
	    "CTrie key = '" << cp.key() << "'" << endl;
      } else if (rp != refMap.end() && cp == cmap.end()) {
	cout << "ERROR: upper_bound with match part failed: " <<
	    "substring = '" << substr << "', " <<
	    "ref key = '" << rp->first << "', " <<
	    "CTrie key at end" << endl;
      } else if (rp->first != cp.key()) {
	cout << "ERROR: upper_bound with match part failed: " <<
	    "substring = '" << substr << "', " <<
	    "ref key = '" << rp->first << "', " <<
	    "CTrie key = '" << cp.key() << "'" << endl;
      }

      range = cmap.equal_range(substr, true);
      if (refLower->first != range.first.key()) {
	cout << "ERROR: lower equal range with match part failed: " <<
	    "substring = '" << substr << "', " <<
	    "ref key = '" << refLower->first << "', " <<
	    "CTrie key = '" << range.first.key() << "'" << endl;
      }
      if (rp == refMap.end() && range.second == cmap.end()) {
	;          // Ok - both are at end
      } else if (rp == refMap.end() && range.second != cmap.end()) {
	cout << "ERROR: upper equal range with match part failed: " <<
	    "substring = '" << substr << "', " << "ref key at end, " <<
	    "CTrie key = '" << range.second.key() << "'" << endl;
      } else if (rp != refMap.end() && range.second == cmap.end()) {
	cout << "ERROR: upper equal range with match part failed: " <<
	    "Substring = '" << substr << "', " <<
	    "ref key = '" << rp->first << "', " << "CTrie key at end" << endl;
      } else if (rp->first != range.second.key()) {
	cout << "ERROR: upper equal range with match part failed: " <<
	    "substring = '" << substr << "', " <<
	    "ref key = '" << rp->first << "', " <<
	    "CTrie key = '" << range.second.key() << "'" << endl;
      }
    }
  }

  cout << "Checking lookups with random strings\n";
  srand(1);
  for (int word_num = 0; word_num < 100000; ++word_num) {
//    if (word_num > 0 && word_num % 1000 == 0)
//      cout << "Doing iteration " << word_num << endl;
    size_t length = uintRand(14) + 1;
    string randomStr;
    for (size_t i = 0; i < length; ++i) {
      randomStr += static_cast<char>(uintRand(28) + '@');
    }

    rp = refMap.find(randomStr);
    cp = cmap.find(randomStr);
    if (rp == refMap.end() && cp == cmap.end()) {
      ; // OK
    } else if (rp == refMap.end() && cp != cmap.end()) {
      cout << "ERROR: find() on '" << randomStr << "' failed: " <<
	  "ref key at end, " << "CTrie key = '" << cp.key() << "'" << endl;
    } else if (rp != refMap.end() && cp == cmap.end()) {
      cout << "ERROR: find() on '" << randomStr << "' failed: " <<
	  "ref key = '" << rp->first << "', CTrie key at end" << endl;
    } else if (rp->first != cp.key()) {
      cout << "ERROR: find() on '" << randomStr << "' failed: " <<
	  "ref key = '" << rp->first << "', " <<
	  "CTrie key = '" << cp.key() << "'" << endl;
    }

    if (cmap.count(randomStr) != refMap.count(randomStr)) {
      cout << "ERROR: count() mismatched on '" << randomStr << "'" << endl;
    }

    rp = refMap.lower_bound(randomStr);
    cp = cmap.lower_bound(randomStr);
    if (rp == refMap.end() && cp == cmap.end()) {
      ; // OK
    } else if (rp == refMap.end() && cp != cmap.end()) {
      cout << "ERROR: lower_bound() on '" << randomStr << "' failed: " <<
	  "ref key at end, " << "CTrie key = '" << cp.key() << "'" << endl;
    } else if (rp != refMap.end() && cp == cmap.end()) {
      cout << "ERROR: lower_bound() on '" << randomStr << "' failed: " <<
	  "ref key = '" << rp->first << "', CTrie key at end" << endl;
    } else if (rp->first != cp.key()) {
      cout << "ERROR: lower_bound() on '" << randomStr << "' failed: " <<
	  "ref key = '" << rp->first << "', " <<
	  "CTrie key = '" << cp.key() << "'" << endl;
    }

    rp = refMap.upper_bound(randomStr);
    cp = cmap.upper_bound(randomStr);
    if (rp == refMap.end() && cp == cmap.end()) {
      ; // OK
    } else if (rp == refMap.end() && cp != cmap.end()) {
      cout << "ERROR: upper_bound() on '" << randomStr << "' failed: " <<
	  "ref key at end, " << "CTrie key = '" << cp.key() << "'" << endl;
    } else if (rp != refMap.end() && cp == cmap.end()) {
      cout << "ERROR: upper_bound() on '" << randomStr << "' failed: " <<
	  "ref key = '" << rp->first << "', CTrie key at end" << endl;
    } else if (rp->first != cp.key()) {
      cout << "ERROR: upper_bound() on '" << randomStr << "' failed: " <<
	  "ref key = '" << rp->first << "', " <<
	  "CTrie key = '" << cp.key() << "'" << endl;
    }
  }

  // Try inserting every key again, after copying the cmap
  cout << "Checking inserts of already existing keys\n";
  IntCTrie cmap2 = cmap;
  for (rp = refMap.begin(); rp != refMap.end(); ++rp) {
    pair<IntCTrie::iterator, bool> rtn = cmap2.insert(rp->first, -1);
    if (rtn.second) {
      cout << "ERROR: Return value for insert of " << rp->first <<
	  " indicates success when it should have failed" << endl;
    }
    if (rtn.first.key() != rp->first) {
      cout << "ERROR: Key of return iterator for insert of duplicate key " <<
	  rp->first << " is wrong: " << rtn.first.key() << endl;
    }
    if (*rtn.first != rp->second) {
      cout << "ERROR: Value of return iterator for insert of duplicate key " <<
	  rp->first << " should be " << rp->second <<
	  " and is " << *rtn.first << endl;
    }
    if (cmap2.size() != refMap.size()) {
      cout << "ERROR: Size of CTrie map should be " << refMap.size() <<
	  " after insert of duplicate key " << rp->first <<
	  " and is " << cmap2.size() << endl;
    }
  }

  cout << "Checking that all entries are in the duplicate map\n";
  rp = refMap.begin();
  cp = cmap2.begin();
  wordCount = 0;
  for (; rp != refMap.end() && !cp.at_end(); ++rp, ++cp, ++wordCount) {
//    cout << "Comparing '" << rp->first << "' (#" << rp->second << ")\n";
    if (rp->first != cp.key()) {
      cout << "ERROR: Keys differ on entry " << wordCount << ": " <<
	  "ref key = '" << rp->first << "', " <<
	  "CTrie key = '" << cp.key() << "'" << endl;
    }
    if (rp->second != *cp) {
      cout << "ERROR: Values differ on entry " << wordCount <<
	  "(key=" << rp->first << "): ref value = '" << rp->second << "', " <<
	  "CTrie value = '" << *cp << "'" << endl;
    }
  }
  if (rp != refMap.end()) {
    cout << "ERROR: Got to end of CTrie " <<
	"but not to the end of the ref map" << endl;
  }
  if (!cp.at_end()) {
    cout << "ERROR: Got to end of the ref map " <<
	"but not to the end of the CTrie" << endl;
  }

  cout << "Checking clear()\n";
  cmap2.clear();
  if (cmap2.size() != 0) {
    cout << "ERROR: CTrie size not zero after clear()" << endl;
  }
  for (cp = cmap2.begin(), wordCount = 0; !cp.at_end(); ++cp, ++wordCount) {}

  if (wordCount != 0) {
    cout << "ERROR: CTrie numEntries not zero after clear()" << endl;
  }

  cout << "Checking random erase() and insert()'s\n";
  cmap2 = cmap;
  vector<string> keys;
  transform(refMap.begin(), refMap.end(), back_inserter(keys),
	  [](const map<string,int>::value_type& v) {return v.first;});
  cout << "There are " << keys.size() << " keys\n";
  random_shuffle(keys.begin(), keys.end());
  vector<string>::iterator insertKey = keys.begin();
  vector<string>::iterator eraseKey = keys.begin();
  size_t cmap_size = refMap.size();
  while (eraseKey != keys.end()) {
    for (size_t i = 0; i < 10 && eraseKey != keys.end(); ++i, ++eraseKey) {
//      cout << "Deleting key '" << *eraseKey << "'" << endl;
      size_t rtn = cmap2.erase(*eraseKey);
      if (rtn != 1) {
	cout << "ERROR: Return value for erase of " << *eraseKey <<
	    " should be 1 and is " << rtn << endl;
      }
      --cmap_size;
    }
    for (size_t i = 0; i < 5 && insertKey != keys.end(); ++i, ++insertKey) {
//      cout << "Reinserting key '" << *insertKey << "'" << endl;
      pair<IntCTrie::iterator, bool> rtn =
	  cmap2.insert(*insertKey, refMap[*insertKey]);
      ++cmap_size;
      if (!rtn.second) {
	cout << "ERROR: Return value for insert of " << *insertKey <<
	    " indicates failure" << endl;
      }
      if (rtn.first.key() != *insertKey) {
	cout << "ERROR: Key of return iterator for insert of " << *insertKey <<
	    " is wrong: " << rtn.first.key() << endl;
      }
      if (*rtn.first != refMap[*insertKey]) {
	cout << "ERROR: Value of return iterator for insert of " <<
	    *insertKey << " should be " << refMap[*insertKey] <<
	    " and is " << *rtn.first << endl;
      }
      if (cmap2.size() != cmap_size) {
	cout << "ERROR: Size of CTrie map should be " << cmap_size <<
	    " after insert of " << *insertKey << " and is " << cmap2.size() <<
	    endl;
      }
    }

    vector<string>::iterator checkKey;
    for (checkKey = keys.begin(); checkKey != insertKey; ++checkKey) {
      cp = cmap2.find(*checkKey);
      if (cp == cmap2.end()) {
	cout << "ERROR: find() failed on entry erased and reinserted: " <<
	    "key = '" << *checkKey << "'" << endl;
      } else if (cp.key() != *checkKey) {
	cout << "ERROR: On entry erased and reinserted, " <<
	    "key of find() iterator is wrong: key = '" << *checkKey << "', " <<
	    "iterator key = '" << cp.key() << "'" << endl;
      } else if (*cp != refMap[*checkKey]) {
	cout << "ERROR: On entry erased and reinserted, " <<
	    "value of find() iterator is wrong: " <<
	    "value = '" << refMap[*checkKey] << "', " <<
	    "CTrie value = '" << *cp << "'" << endl;
      }
    }

    for (; checkKey != eraseKey; ++checkKey) {
      cp = cmap2.find(*checkKey);
      if (cp != cmap2.end()) {
	cout << "ERROR: find() found an entry that was erased: " <<
	    "key = '" << *checkKey << "', " <<
	    "iterator key = '" << cp.key() << "'" << endl;
      }
    }

    for (; checkKey != keys.end(); ++checkKey) {
      cp = cmap2.find(*checkKey);
      if (cp == cmap2.end()) {
	cout << "ERROR: find() failed on untouched entry: " <<
	    "key = '" << *checkKey << "'" << endl;
      } else if (cp.key() != *checkKey) {
	cout << "ERROR: On untouched entry, key of find() iterator is wrong: "<<
	    "key = '" << *checkKey << "', " <<
	    "iterator key = '" << cp.key() << "'" << endl;
      } else if (*cp != refMap[*checkKey]) {
	cout << "ERROR: On untouched entry, " <<
	    "value of find() iterator is wrong: " <<
	    "value = '" << refMap[*checkKey] << "', " <<
	    "CTrie value = '" << *cp << "'" << endl;
      }
    }
  }

  cout << "Erasing rest of keys in map\n";
  eraseKey = keys.begin();
  while (eraseKey != insertKey) {
    for (size_t i = 0; i < 10 && eraseKey != insertKey; ++i, ++eraseKey) {
//      cout << "Deleting key '" << *eraseKey << "'" << endl;
      cp = cmap2.find(*eraseKey);
      if (cp.at_end()) {
	cout << "ERROR: Couldn't find key '" << *eraseKey << "' to erase" <<
	    endl;
	return 1;
      }
      ++cp;
      string next_key = (cp == cmap2.end()) ? string() : cp.key();
      --cp;
      cmap2.erase(cp);
      --cmap_size;
      if (cp == cmap2.end() && next_key.empty()) {
	; // OK
      } else if (cp == cmap2.end() && !next_key.empty()) {
	cout << "ERROR: Iterator after erase is at the end, " <<
	    "but it should be " << next_key << endl;
      } else if (cp != cmap2.end() && next_key.empty()) {
	cout << "ERROR: Iterator after erase should be at end, " <<
	    "but is at " << cp.key() << endl;
      } else if (cp.key() != next_key) {
	cout << "ERROR: Iterator after erase of " << *eraseKey << " is " <<
	    "'" << cp.key() << "', but should be '" << next_key << "'" << endl;
      }
    }
    if (cmap2.size() != cmap_size) {
      cout << "ERROR: Size of CTrie map should be " << cmap_size <<
	  " but is " << cmap2.size() << " after erasure" << endl;
    }

    vector<string>::iterator checkKey;
    for (checkKey = keys.begin(); checkKey != eraseKey; ++checkKey) {
      int count = cmap2.count(*checkKey);
      if (count != 0) {
	cout << "ERROR: find() found an entry that was erased: " <<
	    "key = '" << *checkKey << "', count = '" << count << "'" << endl;
      }
    }

    for (; checkKey != insertKey; ++checkKey) {
      cp = cmap2.find(*checkKey);
      if (cp == cmap2.end()) {
	cout << "ERROR: find() failed on entry not yet erased: " <<
	    "key = '" << *checkKey << "'" << endl;
      } else if (cp.key() != *checkKey) {
	cout << "ERROR: On entry not yet erased, " <<
	    "key of find() iterator is wrong: key = '" << *checkKey << "'," <<
	    "iterator key = '" << cp.key() << "'" << endl;
      } else if (*cp != refMap[*checkKey]) {
	cout << "ERROR: On entry not yet erased, " <<
	    "value of find() iterator is wrong: " <<
	    "value = '" << refMap[*checkKey] << "', " <<
	    "CTrie value = '" << *cp << "'" << endl;
      }
    }

    for (; checkKey != keys.end(); ++checkKey) {
      int count = cmap2.count(*checkKey);
      if (count != 0) {
	cout << "ERROR: find() found an entry that was erased: " <<
	    "key = '" << *checkKey << "', count = '" << count << "'" << endl;
      }
    }
  }

  cmap.insert("A", 20000);
  cmap.insert("", 20005);
  refMap.insert(make_pair(string("A"), 20000));
  refMap.insert(make_pair(string(""), 20005));
  const char *testWords[] = {
      "ABSENTEEISM",
      "ABSTRACTIONISTXY",
      "ANTENNA",
      "ABSENTMIND",
      "ABSENTS",
      "ABSENTED",
      "ABSTRACTIONISTS",
      0
  };

  cout << "Checking prefixes" << endl;
  for (const char** testWord = testWords; *testWord; ++testWord) {
//    cout << "Finding prefixes for '" << *testWord << "'" << endl;
    vector<string> expected = getExpectedPrefixes(*testWord, refMap);
    vector<string>::iterator expectedIter = expected.begin();
    for (IntCTrie::prefix_iter p = cmap.prefix_begin(*testWord);
	p != cmap.prefix_end(); ++p) {
//      cout << "Found prefix of '" << p.key() << "' (" << *p << ")" << endl;
      if (expectedIter == expected.end()) {
	cout << "ERROR: Prefix iterator should be at end: " <<
	    "key = '" << p.key() << "'" << endl;
        continue;
      }

      if (p.key() != *expectedIter) {
        cout << "ERROR: Prefix iterator key is wrong: " <<
            "prefix key = '" << p.key() << "', " <<
            "expected key = '" << *expectedIter << "'" << endl;
      }
      if (p.base().key() != *expectedIter) {
        cout << "ERROR: Prefix iterator base key is wrong: " <<
            "prefix base key = '" << p.base().key() << "', " <<
            "expected key = '" << *expectedIter << "'" << endl;
      }
      ++expectedIter;
    }
  }

  cout << "Checking reverse prefixes" << endl;
  for (const char** testWord = testWords; *testWord; ++testWord) {
//    cout << "Finding reverse prefixes for '" << *testWord << "'" << endl;
    vector<string> expected = getExpectedPrefixes(*testWord, refMap);
    vector<string>::reverse_iterator expectedIter = expected.rbegin();
    for (IntCTrie::reverse_prefix_iter p = cmap.prefix_rbegin(*testWord);
	p != cmap.prefix_rend(*testWord); ++p) {
//      cout << "Found reverse prefix of '" << p.key() << "' (" << *p << ")" <<
//        endl;
      if (expectedIter == expected.rend()) {
	cout << "ERROR: Reverse prefix iterator should be at end: " <<
	    "key = '" << p.key() << "'" << endl;
        continue;
      }

      if (p.key() != *expectedIter) {
        cout << "ERROR: Reverse prefix iterator key is wrong: " <<
            "reverse prefix key = '" << p.key() << "', " <<
            "expected key = '" << *expectedIter << "'" << endl;
      }
      ++expectedIter;
    }
  }

  ++cmap["ABSENTEEISM"];
  if (*cmap.find("ABSENTEEISM") != refMap["ABSENTEEISM"] + 1) {
    cout << "ERROR: operator[] with ++ didn't work" << endl;
  }
  cmap["foobar"] = 153;
  cp = cmap.find("foobar");
  if (cp == cmap.end() || *cp != 153) {
    cout << "ERROR: insertion with operator[] didn't work" << endl;
  }

  checkConst(cmap);
  
  return 0;
}

void
checkConst(const IntCTrie& cmap)
{
//  cmap["foobar"] = 0;
  IntCTrie::const_iterator p = cmap.begin();
//  *p = 25;
}
