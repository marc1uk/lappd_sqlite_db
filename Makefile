CXXFLAGS=-g -std=c++1y

PKGS    := ncursesw
NCCFLAGS  := $(shell pkg-config --cflags $(PKGS))
NCLDFLAGS := $(shell pkg-config --libs $(PKGS))
ADDITIONAL_LIBRARIES=
ifeq ($(UNAME_S), Darwin) #APPLE
	ADDITIONAL_LIBRARIES= -framework Cocoa
endif
ImGuiPath := /home/marc/LinuxSystemFiles/imgui
ImTuiPath := $(ImGuiPath)/examples/example_imtui

all: main insert_demo select_demo

# stuff for imgui
build/imgui.cpp.o: $(ImGuiPath)/imgui.cpp
	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath) $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

build/imgui_draw.cpp.o: $(ImGuiPath)/imgui_draw.cpp
	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath) $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

build/imgui_demo.cpp.o: $(ImGuiPath)/imgui_demo.cpp
	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath) $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

build/imgui_widgets.cpp.o: $(ImGuiPath)/imgui_widgets.cpp
	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath) $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

lib/libimgui.a: ./build/imgui.cpp.o ./build/imgui_draw.cpp.o ./build/imgui_demo.cpp.o ./build/imgui_widgets.cpp.o
	ar qc $@ $^
	ranlib lib/libimgui.a

# stuff for imtui
build/imtui-impl-text.cpp.o: $(ImTuiPath)/imtui-impl-text.cpp
	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath)/examples -I$(ImTuiPath) -I$(ImTuiPath)/include/ -MD -MP -c -o $@ $^

lib/libimtui-core.a: build/imtui-impl-text.cpp.o
	ar qc $@ $^
	ranlib  $@

build/imtui-impl-ncurses.cpp.o: $(ImTuiPath)/imtui-impl-ncurses.cpp
	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath)/examples -I$(ImTuiPath)/include/ $(NCLDFLAGS) $(NCCFLAGS) -MD -MP -c -o $@ $^

lib/libimtui-ncurses.a: build/imtui-impl-ncurses.cpp.o
	ar qc $@ $^
	ranlib $@

main: lib/libimgui.a lib/libimtui-core.a lib/libimtui-ncurses.a src/TUI_app.cpp src/DBManager.cpp src/Layer_enum.cpp
	g++ $(CXXFLAGS)  $^ -I./include -I$(ImGuiPath)/examples -I$(ImTuiPath)/include/ lib/libimgui.a lib/libimtui-core.a lib/libimtui-ncurses.a -lcurses -lform -lsqlite3 -ldl -MD -MP -o $@

insert_demo: src/insert_demo.cpp src/DBManager.cpp src/Layer_enum.cpp
	g++ $(CXXFLAGS) $^ -o $@ -I ./include -lsqlite3

select_demo: src/select_demo.cpp src/DBManager.cpp src/Layer_enum.cpp
	g++ $(CXXFLAGS) $^ -o $@ -I ./include -lsqlite3
