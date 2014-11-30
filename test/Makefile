CXX         = g++
CXXFLAGS    = -I.. -std=c++0x
DEBUG_FLAGS = -g -Wall -W -Wpointer-arith -Wconversion -Wwrite-strings
CTRIE_SRCS  = ../ctrie.h \
              ../ctrie_base.h \
              ../ctrie_cmpr.h \
              ../ctrie_full.h \
              ../ctrie_leaf.h \
              ../ctrie_main.h

%.o: %.cc
	$(CXX) -O2 $(CXXFLAGS) -c -o $@ $<

%.dbg: %.cc
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -c -o $@ $<

ctrie_tst: ctrie_tst.dbg
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $<

time_ctrie: time_ctrie.o
	$(CXX) -O2 $(CXXFLAGS) -o $@ $<

time_map: time_map.o
	$(CXX) -O2 $(CXXFLAGS) -o $@ $<

clean:
	rm -f *.o *.dbg ctrie_tst time_ctrie time_map

ctrie_tst.dbg: $(CTRIE_SRCS)
time_ctrie.o: $(CTRIE_SRCS)
time_ctrie.dbg: $(CTRIE_SRCS)
