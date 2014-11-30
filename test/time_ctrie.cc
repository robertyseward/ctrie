#include "ctrie.h"

#include <iostream>
#include <map>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

using namespace std;
using namespace ctrie;

typedef CTrie<int,Medium> IntCTrie;

void memoryUsage();

size_t
uintRand(size_t interval)
{
  return static_cast<size_t>(
      static_cast<double>(interval) * rand() / (RAND_MAX + 1.0));
}

bool
myGet(istream& cin, char* buf, size_t buf_size, char delim)
{
  char c;
  while (--buf_size > 0 && !cin.get(c).fail() && c != delim) {
    *buf++ = c;
  }
  *buf = '\0';
  if (!cin.fail() && c == delim) {
    cin.putback(c);
  }
  return !cin.fail();
}

int main()
{
  vector<char*> words;
  IntCTrie tries[1000];
  size_t wordBufSize = 4;
  char *wordBuf = new char[4];
  int wordCount = 0;
  char nextc;

  for (size_t i = 0; i < 1000; ++i) {
    tries[i] = IntCTrie();
  }

  clock_t times[100];

  // Read in all the words and populate the test map and reference map.
  // We make the value the word position in the input file.
  while (1) {
    if (!myGet( cin, wordBuf, wordBufSize, '\n' )) {
      break;
    }
    while (!cin.get(nextc).fail() && nextc != '\n') {
      char *new_word_buf = new char[wordBufSize * 2];
      memcpy(new_word_buf, wordBuf, wordBufSize - 1);
      delete [] wordBuf;
      wordBuf = new_word_buf;
      wordBuf[wordBufSize-1] = nextc;
      myGet(cin, wordBuf+wordBufSize, wordBufSize, '\n');
      wordBufSize *= 2;
    }
    // cout << wordBuf << '\n';
    char *word_copy = new char[strlen(wordBuf) + 1];
    strcpy(word_copy, wordBuf);
    words.push_back(word_copy);
  }
  times[0] = clock();
  for (vector<char*>::const_iterator p = words.begin(); p != words.end(); ++p) {
    ++wordCount;
    for (size_t loop = 0; loop < 100; ++loop) {
      tries[loop].insert(*p, wordCount);
    }
  }
#if 1
//    cout << "There are " << tries[0].nNodes() << " nodes in one trie\n";
  memoryUsage();

  // Iterate through the test and reference map, making sure the keys and
  // values are the same.
  times[1] = clock();
  for (size_t loop = 0; loop < 1000; ++loop) {
    IntCTrie::iterator rp;
    wordCount = 0;
    for (rp = tries[0].begin(); !rp.at_end(); ++rp, ++wordCount) {}
  }
#endif

#if 1
  // Now go through every key and do a find(), lower_bound(), upper_bound(),
  // and equal_range() on both maps.  The results should be the same.
  times[2] = clock();
  string key;
  int sum = 0;
  for (IntCTrie::iterator rp = tries[0].begin(); !rp.at_end(); ++rp) {
    key = rp.key();
    for (size_t loop = 0; loop < 1000; ++loop) {
      sum += *tries[0].find(key);
    }
  }
#endif

#if 1
  times[3] = clock();
  srand(1);
  sum = 0;
  for (size_t wordNum = 0; wordNum < 10000; ++wordNum) {
    size_t length = uintRand(14) + 1;
    string randomStr;
    for (size_t i = 0; i < length; ++i) {
      randomStr += static_cast<char>(uintRand(28) + '@');
    }

    IntCTrie::iterator rp;
    for (size_t loop = 0; loop < 1000; ++loop) {
      rp = tries[0].find(randomStr);
      if (!rp.at_end()) {
        sum += *rp;
      }
    }
  }
  times[4] = clock();

  cout << "Time to create 100 maps: " << (times[1]-times[0])/1000 << " ms\n";
  cout << "Time to iterate 1000 times: " << (times[2]-times[1])/1000 << " ms\n";
  cout << "Time to find all keys 1000 times: " <<
      (times[3]-times[2])/1000 << " ms\n";
  cout << "Time to find 10000 random words 1000 times: " <<
      (times[4]-times[3])/1000 << " ms\n";
#endif
  return 0;
}

#define _INCLUDE_POSIX_SOURCE
#define _INCLUDE_XOPEN_SOURCE_EXTENDED

#include "sys/resource.h"
#include "malloc.h"

void memoryUsage()
{
  struct rusage R;
  struct mallinfo m = mallinfo();
  cout << "Malloc info: arenaSpace = " << m.arena << ", " <<
      "smallSpace = " << m.usmblks << ", " <<
      "ordSpace = " << m.uordblks << "\n";
  getrusage(0, &R);
  cout << "memory usage: shared_mem = " << R.ru_ixrss << ", " <<
     "ushared_data = " << R.ru_idrss << ", " <<
     "ushared_stack = " << R.ru_isrss << ", " <<
     "swaps = " << R.ru_nswap << ", " <<
     "page_faults = " << R.ru_majflt << "\n";
}
