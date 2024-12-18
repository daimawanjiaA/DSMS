PROJ   = ../../bin/client
LPRJ   = ../../lib/libclient.a
OBJS   = $(patsubst %.cpp, %.o, $(wildcard ../common/*.cpp *.cpp))
LOBJ   = $(filter-out %main.o, $(OBJS))
CC     = g++
LINK   = g++
AR     = ar rv
RM     = rm -rf 
CFLAGS = -c -Wall \
         -I/usr/include/acl-lib/acl -I/usr/include/acl-lib/acl_cpp \
         -I../common \
		 -g -F dwarf
LIBS   = -pthread -lacl_all

all: $(PROJ) $(LPRJ)

$(PROJ): $(OBJS)
	$(LINK) $^ $(LIBS) -o $@

$(LPRJ): $(LOBJ)
	$(AR) $@ $^

.cpp.o:
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(PROJ) $(LPRJ) $(OBJS)