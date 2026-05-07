CPPFLAGS=-g -std=c++1y
all: get_storage_type insert_demo select_demo

#main:

get_storage_type: get_storage_type.cpp Layer_enum.cpp
	g++ $(CPPFLAGS) $^ -o $@ -I ./ -lsqlite3

insert_demo: insert_demo.cpp DBManager.cpp Layer_enum.cpp
	g++ $(CPPFLAGS) $^ -o $@ -I ./ -lsqlite3

select_demo: select_demo.cpp DBManager.cpp Layer_enum.cpp
	g++ $(CPPFLAGS) $^ -o $@ -I ./ -lsqlite3
