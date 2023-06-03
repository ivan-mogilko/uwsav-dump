INCDIR = ./
LIBDIR = 
TARGET = uwsav-dump

CC ?= gcc
CXX ?= g++
CFLAGS := -fvisibility=hidden -O2 -g \
        -Werror=write-strings -Werror=format -Werror=format-security \
        -DNDEBUG \
	$(CFLAGS)
CXXFLAGS := -std=c++14 -fpermissive \
	$(CXXFLAGS)

CFLAGS   := $(addprefix -I,$(INCDIR)) $(CFLAGS)
CXXFLAGS := $(CFLAGS) $(CXXFLAGS)
LDFLAGS = $(addprefix -L,$(LIBDIR))


OBJS_UTILS = \
	utils/compat_stdio.c \
	utils/filestream.cpp

OBJS_UWSAV = \
	uwsav/uwsav_data.cpp \
	uwsav.cpp

OBJS := $(OBJS_UTILS) $(OBJS_UWSAV)

OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.cc=.o)
OBJS := $(OBJS:.cpp=.o)


.PHONY: all printflags printobjs rebuild clean

all: printflags $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking..."
	@$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

%.o: %.c
	@echo $@
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	@echo $@
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cc
	@echo $@
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

printflags:
	@echo "CFLAGS =" $(CFLAGS) "\n"
	@echo "CXXFLAGS =" $(CXXFLAGS) "\n"
	@echo "LDFLAGS =" $(LDFLAGS) "\n"

printobjs:
	@echo $(OBJS)

rebuild: clean all

clean:
	@echo "Cleaning..."
	@rm -f $(TARGET)

