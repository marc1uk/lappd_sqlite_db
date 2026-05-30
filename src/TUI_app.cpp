#include "imtui/imtui.h"
#include "imtui/imtui-impl-ncurses.h"
#include <iostream>
#include "DBManager.h"

namespace ImGui {
	static void HelpMarker(const char* desc);
}

struct LAPPDConfiguration {
	Placement placement;
	std::string status;
	LAPPD lappd;
	ACDC acdc1;
	ACDC acdc2;
	void clear(){
		placement.clear();
		status.clear();
		lappd.clear();
		acdc1.clear();
		acdc2.clear();
	}
};

int main() {
	
	// ImTui initialisation
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto screen = ImTui_ImplNcurses_Init(true);
	ImTui_ImplText_Init();
	
	ImGuiWindowFlags window_flags = 0;
	bool no_titlebar=false, no_menu=false, no_scrollbar=false;
	if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
	if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	
	int nlines=40;
	int ncolumns=169;
	char* clines = getenv("LINES");
	char* ccolumns = getenv("COLUMNS");
	if(clines) nlines=atoi(clines);
	if(ccolumns) ncolumns=atoi(ccolumns);
	
	// sqlite initialisation
	std::string db_file = "lappd_db.sqlite";
	DBManager db(db_file);
	
	bool app_open = true;
	while (app_open) {
		
		// start next frame
		ImTui_ImplNcurses_NewFrame();
		ImTui_ImplText_NewFrame();
		ImGui::NewFrame();
		
		// configure a default position/size in case there's no data in the .ini file.
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(ncolumns, nlines), ImGuiCond_Once);
		
		////////////////////
		// begin main window
		if(!ImGui::Begin("LAPPD Database Application", &app_open, window_flags)){
			ImGui::End();
			return 0;
		}
		
		// set of tabs to show views: by run, and by LAPPD
		if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None)){
			
			if (ImGui::BeginTabItem("View By Runs")){
				
				static std::vector<RunRange> run_ranges = db.storage.get_all<RunRange>();
				
				// make a refresh button? TODO also invoke on switching tabs?
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
				if(ImGui::Button("Refresh")) run_ranges = db.storage.get_all<RunRange>();
				ImGui::PopStyleColor(3);
				
				//ImGui::SetNextItemWidth(ImGui::GetFontSize() * 18);
				//ImGui::SetNextItemWidth(100);
				
				// make a table - one row for each runrange, plus one header row
				for(int i=0; i<run_ranges.size()+1; ++i){
					ImGui::PushID(i);
					
					ImGui::Columns(6, NULL, false);
					static std::vector<int> col_widths{15, 22, 15, 22, 25, 50}; // FIXME better not to hard-code, but... needs to fit content
					for(int j=0; j<6; ++j){
						ImGui::SetColumnWidth(j, col_widths[j]);
					}
					if(i==0){
						ImGui::Text("start run");
						ImGui::NextColumn();
						ImGui::Text("start date");
						ImGui::NextColumn();
						ImGui::Text("end run");
						ImGui::NextColumn();
						ImGui::Text("end date");
						ImGui::NextColumn();
						ImGui::Text("comments");
						ImGui::NextColumn();
						ImGui::Text("LAPPDs");
					} else {
						RunRange& rr = run_ranges.at(i-1);
						ImGui::Text("%d",rr.start_run);
						ImGui::NextColumn();
						ImGui::Text("%s",rr.start_date.c_str());
						ImGui::NextColumn();
						ImGui::Text("%d",rr.end_run);
						ImGui::NextColumn();
						ImGui::Text("%s",rr.end_date.c_str());
						ImGui::NextColumn();
						ImGui::TextWrapped("%s",rr.comments.c_str());
						ImGui::NextColumn();
						// then an expandable section with more info
						if(rr.placements.empty()){
							ImGui::Text("No LAPPDs");
						} else {
							// an expandable section showing info about the placements within this run
							static std::string collapse_label;
							collapse_label = std::to_string(rr.placements.size())+" LAPPDs";
							if (ImGui::CollapsingHeader(collapse_label.c_str())){
								
								// we won't show all placements, just one
								// with a pair of <|> arrows with which to scroll them
								static LAPPDConfiguration shown_lappd;
								static int lappd_index = 0;
								// is this going to mess up if a user expands multiple rows?
								// maybe we need to select the appropriate LAPPD for each row each frame?
								// or restrict it to only show one row at a time?
								static auto GetLAPPD = [&db](RunRange& rr){
									auto it = std::next(rr.placements.begin(),lappd_index);
									bool ok = db.GetPlacement(it->first, shown_lappd.placement);
									if(!ok) goto bad_read;
									ok = db.GetLAPPD(shown_lappd.placement.LAPPD_rowID, shown_lappd.lappd);
									if(!ok) goto bad_read;
									ok = db.GetACDC(shown_lappd.lappd.ACDC1_rowID, shown_lappd.acdc1);
									if(!ok) goto bad_read;
									ok = db.GetACDC(shown_lappd.lappd.ACDC2_rowID, shown_lappd.acdc2);
									if(!ok) goto bad_read;
									shown_lappd.status = it->second;
									return;
									
									bad_read:
									shown_lappd.clear();
									return;
								};
								GetLAPPD(rr);
								
								ImGui::Text(""); // i dunno why it doesn't align properly without this and that irks me
								ImGui::SameLine();
								if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
									if(lappd_index>0){
										lappd_index--;
										GetLAPPD(rr);
									}
								}
								static float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
								ImGui::SameLine(0.0f, spacing);
								if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
									if(lappd_index<rr.placements.size()-1){
										lappd_index++;
										GetLAPPD(rr);
									}
								}
								ImGui::SameLine();
								ImGui::Text("%d/%d", lappd_index+1, rr.placements.size());
								ImGui::Text("INCOM ID %d", shown_lappd.lappd.INCOM_ID);
								ImGui::Text("Status: %s", shown_lappd.status.c_str());
								ImGui::Text("Data ID: %d", shown_lappd.placement.Data_ID);
								ImGui::Text("Slow Control ID: %d", shown_lappd.placement.SC_ID);
								ImGui::Text("PPS Ratio: %d", shown_lappd.placement.PPS_Ratio);
								ImGui::Text("Mailbox: %d", shown_lappd.placement.mailbox);
								ImGui::Text("Layer: %d", shown_lappd.placement.layer);
								ImGui::Text("Position: (%.2f, %.2f, %.2f)", shown_lappd.placement.x, shown_lappd.placement.y, shown_lappd.placement.z);
								ImGui::Text("ACC: %d", shown_lappd.placement.ACC_ID);
								ImGui::Text("ACDC 0: %d using pedestals %s, ACC port %d", shown_lappd.acdc1.ID, shown_lappd.acdc1.pedestals.c_str(), shown_lappd.placement.ACDC1_ACCport1);
								ImGui::Text("ACDC 1: %d using pedestals %s, ACC port %d", shown_lappd.acdc2.ID, shown_lappd.acdc2.pedestals.c_str(), shown_lappd.placement.ACDC1_ACCport2);
								ImGui::Text("Comments: %s", shown_lappd.placement.comments.c_str());
							}
						}
					}
					
					ImGui::Columns(1);
					ImGui::PopID();
				}
				
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem("View By LAPPD")){
				// TODO
				ImGui::SetNextItemWidth(200);
				ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
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


namespace ImGui {
	// Helper to display a little (?) mark which shows a tooltip when hovered.
	// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
	void HelpMarker(const char* desc){
		SameLine();
		TextDisabled("(?)");
		if (IsItemHovered()){
			BeginTooltip();
			PushTextWrapPos(GetFontSize() * 35.0f);
			TextUnformatted(desc);
			PopTextWrapPos();
			EndTooltip();
		}
		//if (IsItemHovered()) SetTooltip(desc); < simple edition
	}
}

