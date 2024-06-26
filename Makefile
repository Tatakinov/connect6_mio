LIBRARY			= mio.dll
CXX					= clang++
CXXFLAGS		= -I . -Wall -Ofast -std=c++17 -DNDEBUG
LD					= clang++
LDFLAGS			= -shared -lmsvcrt -Xlinker /NODEFAULTLIB:LIBCMT
OBJS				= connect6.o \
					  lib.o \
					  util.o \
					  windows/dll.o \
					  base/header.o
ALL					= all

.SUFFIXES: .cc .o

.PHONY: all
$(ALL): $(LIBRARY)

$(LIBRARY): $(OBJS)
	$(LD) $(LDFLAGS) -o $(LIBRARY) $(OBJS)

.cc.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -f *.o windows/*.o base/*.o *.exp *.lib *.exe *.dll
