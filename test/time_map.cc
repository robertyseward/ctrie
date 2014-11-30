#include <cstring>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

using namespace std;

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
  map<string,int> maps[1000];
  int wordCount = 0;
  clock_t times[100];

  // Read in all the words and populate the test map and reference map.
  // We make the value the word position in the input file.  Note that
  // we force each map to have its own copy of the string - to do otherwise
  // would be cheating for the map.
  size_t wordBufSize = 4;
  char *wordBuf = new char[4];
  char nextc;

  while (1) {
    if (!myGet(cin, wordBuf, wordBufSize, '\n')) {
      break;
    }
    while (!cin.get(nextc).fail() && nextc != '\n') {
      char *newWordBuf = new char[wordBufSize * 2];
      memcpy(newWordBuf, wordBuf, wordBufSize - 1);
      delete [] wordBuf;
      wordBuf = newWordBuf;
      wordBuf[wordBufSize - 1] = nextc;
      myGet(cin, wordBuf + wordBufSize, wordBufSize, '\n');
      wordBufSize *= 2;
    }
//	cout << wordBuf << '\n';
    char *wordCopy = new char[strlen(wordBuf) + 1];
    strcpy(wordCopy, wordBuf);
    words.push_back(wordCopy);
  }
  times[0] = clock();
  for (vector<char*>::const_iterator p = words.begin(); p != words.end(); ++p) {
    ++wordCount;
    for (int loop = 0; loop < 100; ++loop) {
      maps[loop].insert(make_pair(string(*p), wordCount));
    }
  }
  memoryUsage();

  // Iterate through the test and reference map, making sure the keys and
  // values are the same.
  times[1] = clock();
  for (int loop = 0; loop < 1000; ++loop) {
    map<string,int>::iterator rp;
    wordCount = 0;
    for (rp = maps[0].begin(); rp != maps[0].end(); ++rp, ++wordCount) {}
  }

  // Now go through every key and do a find(), lower_bound(), upper_bound(),
  // and equal_range() on both maps.  The results should be the same.
  times[2] = clock();
  for (int loop = 0; loop < 1000; ++loop) {
    int sum = 0;
    map<string,int>::iterator rp;
    for (rp = maps[0].begin(); rp != maps[0].end(); ++rp) {
      sum += maps[0].find(rp->first)->second;
    }
  }

  times[3] = clock();
  srand(1);
  int sum = 0;
  for (int word_num = 0; word_num < 10000; ++word_num) {
    size_t length = uintRand(14) + 1;
    string randomStr;
    for (size_t i = 0; i < length; ++i) {
      randomStr += static_cast<char>(uintRand(28) + '@');
    }

    for (int loop = 0; loop < 1000; ++loop) {
      map<string,int>::iterator rp = maps[0].find(randomStr);
      if (rp != maps[0].end()) {
        sum += rp->second;
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
    cout << "Malloc info: arenaSpace = " << m.arena << 
	    ", smallSpace = " << m.usmblks <<
	    ", ordSpace = " << m.uordblks << "\n";
    getrusage(0, &R);
    cout << "memory usage: shared_mem = " << R.ru_ixrss <<
	   ", ushared_data = " << R.ru_idrss <<
	   ", ushared_stack = " << R.ru_isrss <<
	   ", swaps = " << R.ru_nswap <<
	   ", page_faults = " << R.ru_majflt << "\n";
}

