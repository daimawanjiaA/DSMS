PROJ   = ../../bin/tracker
# patsubst是关键字，表示模式替换，将../common/*.cpp和*.cpp中的.cpp替换成.o；wildcard是关键字，表示通配符，表示所有的.cpp文件，得到的是一个列表
OBJS   = $(patsubst %.cpp, %.o, $(wildcard ../common/*.cpp *.cpp))
CC     = g++
LINK   = g++
RM     = rm -rf
CFLAGS = -c -Wall \
         -I/usr/include/acl-lib/acl_cpp \
         `mysql_config --cflags` \
         -I../common \
		 -g -F dwarf
LIBS   = -pthread -lacl_all `mysql_config --libs`

all: $(PROJ) 

# $@表示目标文件（这里是PROJ），$^表示所有的依赖文件（这里是OBJS）
$(PROJ): $(OBJS)
	$(LINK) $^ $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(PROJ) $(OBJS)