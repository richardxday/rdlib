
RDLIB_CFLAGS=$(shell pkg-config --cflags rdlib-0.1 | sed -E 's/\\\"/\"/g')
RDLIB_LDFLAGS=$(shell pkg-config --libs rdlib-0.1)

CFLAGS=-Wall -Werror -Wextra -msse2 -O0 -g
LDFLAGS=

ifdef MINGW
COMPILER=x86_64-w64-mingw32-
else
COMPILER=
endif

# C compiler
CC = $(COMPILER)gcc
# C++ compiler
CXX = $(COMPILER)g++

APPLICATIONS=datetest xmltest formattest regextest

all: $(APPLICATIONS)

clean:
	-rm -f $(APPLICATIONS)

%: %.cpp
	$(CXX) $(RDLIB_CFLAGS) $(CFLAGS) $< $(LDFLAGS) $(RDLIB_LDFLAGS) -o $@
