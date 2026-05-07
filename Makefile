CPPFLAGS=-g -std=c++1y
all: insert_demo select_demo

#main:

insert_demo: insert_demo.cpp DBManager.cpp Layer_enum.cpp
	g++ $(CPPFLAGS) $^ -o $@ -I ./ -lsqlite3

select_demo: select_demo.cpp DBManager.cpp Layer_enum.cpp
	g++ $(CPPFLAGS) $^ -o $@ -I ./ -lsqlite3
