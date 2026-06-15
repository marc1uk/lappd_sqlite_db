CXXFLAGS=-g -std=c++1y -O3

PKGS    := ncursesw
NCCFLAGS  := $(shell pkg-config --cflags $(PKGS))
NCLDFLAGS := $(shell pkg-config --libs $(PKGS))
ADDITIONAL_LIBRARIES=
ifeq ($(UNAME_S), Darwin) #APPLE
	ADDITIONAL_LIBRARIES= -framework Cocoa
endif
ImGuiPath := /home/marc/LinuxSystemFiles/imgui
ImTuiPath := /home/marc/LinuxSystemFiles/imtui

all: main insert_demo select_demo

## stuff for imgui
#build/imgui.cpp.o: $(ImGuiPath)/imgui.cpp
#	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath) $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

#build/imgui_draw.cpp.o: $(ImGuiPath)/imgui_draw.cpp
#	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath) $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

#build/imgui_demo.cpp.o: $(ImGuiPath)/imgui_demo.cpp
#	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath) $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

#build/imgui_widgets.cpp.o: $(ImGuiPath)/imgui_widgets.cpp
#	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath) $(ADDITIONAL_LIBRARIES) -MD -MP -c -o $@ $^

#lib/libimgui.a: ./build/imgui.cpp.o ./build/imgui_draw.cpp.o ./build/imgui_demo.cpp.o ./build/imgui_widgets.cpp.o
#	ar qc $@ $^
#	ranlib lib/libimgui.a

## stuff for imtui
## n.b. imtui embeds imgui as a submodule, so expects imgui.h in a folder called 'imgui'
## so i'm passing '-I$(ImGuiPath)/..' here, but note this requires imgui is indeed in a folder called 'imgui'
#build/imtui-impl-text.cpp.o: $(ImTuiPath)/src/imtui-impl-text.cpp
#	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath)/.. -I$(ImTuiPath)/include -MD -MP -c -o $@ $^

#lib/libimtui-core.a: build/imtui-impl-text.cpp.o
#	ar qc $@ $^
#	ranlib  $@

#build/imtui-impl-ncurses.cpp.o: $(ImTuiPath)/src/imtui-impl-ncurses.cpp
#	g++ $(CXXFLAGS) -fPIC -I$(ImGuiPath)/.. -I$(ImTuiPath)/include $(NCLDFLAGS) $(NCCFLAGS) -MD -MP -c -o $@ $^

#lib/libimtui-ncurses.a: build/imtui-impl-ncurses.cpp.o
#	ar qc $@ $^
#	ranlib $@

#$(ImTuiPath)/build/third-party/CMakeFiles/imgui-for-imtui.dir/imgui/imgui/imgui_draw.cpp.o
#$(ImTuiPath)/build/third-party/CMakeFiles/imgui-for-imtui.dir/imgui/imgui/imgui_widgets.cpp.o
#CMakeFiles/imtui-ncurses.dir/imtui-impl-ncurses.cpp.o
#CMakeFiles/imtui.dir/imtui-impl-text.cpp.o
#CMakeFiles/imtui-examples-common.dir/imtui-demo.cpp.o


# $(ImTuiPath)/examples folders included here to define InTui::ShowDemoWindow
main: lib/libimgui.a lib/libimtui-core.a lib/libimtui-ncurses.a src/TUI_app.cpp src/DBManager.cpp src/Layer_enum.cpp
	# thi works but results in a very subtle but annoying bug with combo boxes
	#g++ $(CXXFLAGS)  $^ -I./include -I$(ImGuiPath)/.. -I$(ImTuiPath)/include lib/libimgui.a lib/libimtui-core.a lib/libimtui-ncurses.a -lcurses -lform -lsqlite3 -ldl -MD -MP -o $@
	# we have to replace all 3 libraries for this to work at all, in which case it all works completely.
	g++ $(CXXFLAGS)  $^ -I./include -I$(ImGuiPath)/.. -I$(ImTuiPath)/include $(ImTuiPath)/build/third-party/libimgui-for-imtui.a $(ImTuiPath)/build/src/libimtui.a $(ImTuiPath)/build/src/libimtui-ncurses.a -lcurses -lform -lsqlite3 -ldl -MD -MP -o $@

main_gui: lib/libimgui.a src/GUI_app.cpp
	g++ $(CXXFLAGS) $^ -I./include -I$(ImGuiPath) -I$(ImGuiPath)/backends -lform -ldl -MD -MP -o $@

insert_demo: src/insert_demo.cpp src/DBManager.cpp src/Layer_enum.cpp
	g++ $(CXXFLAGS) $^ -o $@ -I ./include -lsqlite3

select_demo: src/select_demo.cpp src/DBManager.cpp src/Layer_enum.cpp
	g++ $(CXXFLAGS) $^ -o $@ -I ./include -lsqlite3

clean:
	rm -rf build/*
	rm -rf main
	rm -rf select_demo
	rm -rf insert_demo
