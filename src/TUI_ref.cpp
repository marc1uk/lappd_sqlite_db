#include "imtui/imtui.h"
#include "imtui/imtui-impl-ncurses.h"
static void HelpMarker(const char* desc);

int main() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	auto screen = ImTui_ImplNcurses_Init(true);
	ImTui_ImplText_Init();
	
	bool app_open = true; // must start true to work
	bool demo = false;
	int nframes = 0;
	float fval = 1.23f;
	int dval=3;
	
	int nlines=40;
	int ncolumns=169;
	char* clines = getenv("LINES");
	char* ccolumns = getenv("COLUMNS");
	if(clines) nlines=atoi(clines);
	if(ccolumns) ncolumns=atoi(ccolumns);
	
	ImGuiWindowFlags window_flags = 0;
	bool no_titlebar=false, no_menu=false, no_scrollbar=false;
	if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
	if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	//window_flags |= ImGuiWindowFlags_NoNav;
	
	while (app_open) {
		
		// start next frame
		ImTui_ImplNcurses_NewFrame();
		ImTui_ImplText_NewFrame();
		ImGui::NewFrame();
		
		// configure a default position/size in case there's no data in the .ini file.
//		ImGui::SetNextWindowPos(ImVec2(4, 2), ImGuiCond_Once);
//		ImGui::SetNextWindowSize(ImVec2(50.0, 10.0), ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(ncolumns, nlines), ImGuiCond_Once);
		
		////////////////////
		// begin main window
		if(!ImGui::Begin("LAPPD Database Application", &app_open, window_flags)){
			ImGui::End();
			return 0;
		}
		
		// checkbox
		ImGui::Checkbox("Done Looking", &app_open); HelpMarker("Close the window.");
		//ImGui::Checkbox("Show Demo", &demo);
		
		// text display
		ImGui::Text("NFrames = %d", nframes++); // acts like printf
		ImGui::Text("Mouse Pos : x = %g, y = %g", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
		ImGui::Text("Time per frame %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
		// bullet pointed text
		ImGui::BulletText("Bullet point 1");
		
		// text value with a label (appears on RHS)
		ImGui::SetNextItemWidth(20); // otherwise label is like, way over.
		ImGui::LabelText("Favourite Fruit", "%s","potato");
		
		// coloured text
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Pink Text"); // HSV colours
		
		// line wrapping textbox - do we not need to use TextWrapped? whats the difference?
		static int wrap_width = 20;
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
		ImGui::Text("The lazy dog is a good dog. This paragraph should fit within %d chars.", wrap_width);
		ImGui::PopTextWrapPos();
		
		// numeric sliders
		ImGui::Text("Sliders:");
		ImGui::SliderFloat("##float", &fval, 0.0f, 10.0f);
		ImGui::SliderInt("##int", &dval, 0.0f, 10.0f);
		
		// radio selection
		static int e = 0;
		ImGui::RadioButton("radio a", &e, 0); ImGui::SameLine();
		ImGui::RadioButton("radio b", &e, 1); ImGui::SameLine();
		ImGui::RadioButton("radio c", &e, 2);
		
		// Loop to create elements using PushID() to add unique identifier in the ID stack
		for (int i = 0; i < 7; i++){
			if (i > 0) ImGui::SameLine();
			ImGui::PushID(i);
			// configure colour
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
			// make a button
			ImGui::Button("Click");
			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}
		
		// buttons with Repeater (hold to re-execute on each frame)
		{
			static int counter = 0;
			float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
			ImGui::PushButtonRepeat(true);
			if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { counter--; }
			ImGui::SameLine(0.0f, spacing);
			if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { counter++; }
			ImGui::PopButtonRepeat();
			ImGui::SameLine();
			ImGui::Text("%d", counter);
		}
		
		ImGui::PushItemWidth(20);
		// combo box - dropdown
		{
			// Using the _simplified_ one-liner Combo() api here
			// See "Combo" section on imgui_demo.cpp for the more complete BeginCombo()/EndCombo() api.
			const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
			static int item_current = 0;
			// 'Choose:' is the label, but remember ImGui puts these on the RHS, not LHS or above.
			//ImGui::Combo("Choose:", &item_current, items, IM_ARRAYSIZE(items));
			ImGui::Text("Choose:"); // so this is probably better. don't use sameline, it messes stuff up
			ImGui::Combo("", &item_current, items, IM_ARRAYSIZE(items));
			// for some reason combo boxes include an unaccounted for empty line at the end
			ImGui::Text("");
		}
		
		// list box - shows all options, you select one
		{
			const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pineapple", "Strawberry", "Watermelon" };
			static int item_current = 1;
			ImGui::Text("Choose One:");
			ImGui::ListBox("", &item_current, items, IM_ARRAYSIZE(items), 4); // note only 4 options shown, others are scrollable
		}
		
		// multiple selectable options
		{
		ImGui::Text("Choose as many as you like:");
		static bool selection[5] = { false, true, false, false, false };
		ImGui::Selectable("1. I am selectable", &selection[0]);
		ImGui::Selectable("2. I am also selectable", &selection[1]);
		ImGui::Selectable("3. we are all selectable", &selection[3]);
		if (ImGui::Selectable("4. double click to select me", selection[4], ImGuiSelectableFlags_AllowDoubleClick))
		if (ImGui::IsMouseDoubleClicked(0)) selection[4] = !selection[4];
		}
		ImGui::PopItemWidth();
		
		// text entry
		{
		static char str0[128] = "Hello, world!";
		ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
		ImGui::SameLine(); HelpMarker("USER:\nHold SHIFT or use mouse to select text.\n" "CTRL+Left/Right to word jump.\n" "CTRL+A or double-click to select all.\n" "CTRL+X,CTRL+C,CTRL+V clipboard.\n" "CTRL+Z,CTRL+Y undo/redo.\n" "ESCAPE to revert");
		
		// To wire InputText() with std::string or any other custom string type,
		// see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/imgui_stdlib.h file.
		static char str1[58] = ""; // default value. Hint is shown when empty.
		ImGui::InputTextWithHint("input text (w/ hint)", "enter text here", str1, IM_ARRAYSIZE(str1));
		ImGui::SameLine(); HelpMarker(
		"USER:\n"
		"Hold SHIFT or use mouse to select text.\n"
		"CTRL+Left/Right to word jump.\n"
		"CTRL+A or double-click to select all.\n"
		"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
		"CTRL+Z,CTRL+Y undo/redo.\n"
		"ESCAPE to revert.\n\n"
		"PROGRAMMER:\n"
		"You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() "
		"to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated "
		"in imgui_demo.cpp).");
		}
		
		// multi-line text input
		static char text[70 * 3] =  "/*\n"
		                               " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
		                               " the hexadecimal encoding of one offending instruction,\n"
		                               " more formally, the invalid operand with locked CMPXCHG8B\n";
		static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
		//ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
		ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(70, ImGui::GetTextLineHeight() * 3), flags);
		
		// numeric entry
		{
		static int i0 = 123;
		ImGui::InputInt("input int", &i0);
		static float f0 = 0.001f;
		ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");
		static double d0 = 999999.00000001;
		ImGui::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");
		}
		
		// arrange elements in columns
		ImGui::Columns(3, NULL, false);
		static bool selected[9] = {};
		for (int i = 0; i < 9; i++){
			char label[20]; sprintf(label, "Item %d", i);
			if (ImGui::Selectable(label, &selected[i])) {}
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		
		// tab group
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)){
			if (ImGui::BeginTabItem("Browse Database")){
				//ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());
				ImGui::SetNextItemWidth(100);
				ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("New Entry")){
				//ImGui::SetNextItemWidth(ImGui::GetFontSize() * 18);
				ImGui::SetNextItemWidth(200);
				ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		
		// collapsable section
		static bool show_section = true; // ImGuiTreeNodeFlags_None for not closable... but it's a bool?
		ImGui::SetNextItemWidth(20);
		if (ImGui::CollapsingHeader("Header", &show_section)){
			for (int i = 0; i < 5; i++) ImGui::Text("Some content %d", i);
		}
		ImGui::Checkbox("Show 2nd header", &show_section); // so we can re-show it
		
		// window menu bar
		if (ImGui::BeginMenuBar()){
			if (ImGui::BeginMenu("File")){
				if(ImGui::MenuItem("Quit", "Close application")) {app_open=false;}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")){
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		
		
		ImGui::End();
		// end main window
		/////////////////
		
//		if(demo) ImTui::ShowDemoWindow(&demo);
//		if(demo) ImGui::ShowDemoWindow(&demo);
		
		ImGui::Render();
		
		ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen);
		ImTui_ImplNcurses_DrawScreen();
	}
	
	ImTui_ImplText_Shutdown();
	ImTui_ImplNcurses_Shutdown();
	
	return 0;
}

/*
            ImGui::IsItemFocused(),
            ImGui::IsItemHovered(),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped),
            ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly),
            ImGui::IsItemActive(),
            ImGui::IsItemEdited(),
            ImGui::IsItemActivated(),
            ImGui::IsItemDeactivated(),
            ImGui::IsItemDeactivatedAfterEdit(),
            ImGui::IsItemVisible(),
            ImGui::IsItemClicked(),
            ImGui::IsItemToggledOpen(),
*/

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
void HelpMarker(const char* desc){
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
	//if (ImGui::IsItemHovered()) ImGui::SetTooltip(desc); < simple edition
}
