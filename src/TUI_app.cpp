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
	//IMGUI_CHECKVERSION(); < we've messed around introducing c++ std::vector support so broke this
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
			// FIXME HIGHLIGHT ACTIVE TAB ON TAB BAR??
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
				
				static std::vector<int> col_widths{15, 22, 15, 22, 25, 50}; // FIXME better not to hard-code, but... needs to fit content
				ImGui::Columns(6, NULL, false);
				for(int j=0; j<6; ++j){
					ImGui::SetColumnWidth(j, col_widths[j]);
				}
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
				
				// to stop appending data to the last column
				ImGui::Columns(1);
				
				// make a row for each runrange
				for(int i=0; i<run_ranges.size(); ++i){
					
					ImGui::PushID(i);
					
					ImGui::Columns(6, NULL, false);
					for(int j=0; j<6; ++j){
						ImGui::SetColumnWidth(j, col_widths[j]);
					}
					
					RunRange& rr = run_ranges.at(i);
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
							GetLAPPD(rr); // this is not ideal, as it reads all this data from disk for every frame!
							// on the other hand, to do it from memory, we'd need to store into memory all placements
							// for all expanded rows (potentially lazy-loaded), so we would need an in-memory map
							// or row index to vector of LAPPDConfigurations.... which could be done, but for now
							// we live with the inefficiency.
							
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
					
					ImGui::Columns(1);
					ImGui::PopID();
				}
				
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem("View By LAPPD")){
				
				// drop-down for selecting which LAPPD's history to show
				static std::vector<std::string> lappds = [&db](){
					std::vector<std::string> ids;
					for(int incom_id : db.storage.select(sqlite_orm::distinct(&LAPPD::INCOM_ID))){
						ids.push_back(std::to_string(incom_id));
					}
					return ids;
				}();
				
				static int current_index = -1;
				ImGui::Text("LAPPD"); ImGui::SameLine();
				ImGui::SetNextItemWidth(20);
				
				// remember interactibles need a unique ID: if we leave the label empty,
				// it gets the same ID as the parent, and then interactions operate on both simultaneously
				// in this case the parent being the tab select button
				// see https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-about-the-id-stack-system
				ImGui::Combo("###lappd_select_combo", &current_index, lappds, 4);
				
				static std::string current_lappd;
				current_lappd = lappds[current_index];
				
				// column headers
				ImGui::Columns(6, NULL, false);
				static std::vector<int> col_widths{15, 22, 15, 22, 25, 50}; // FIXME better not to hard-code, but... needs to fit content
				for(int j=0; j<6; ++j){
					ImGui::SetColumnWidth(j, col_widths[j]);
				}
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
				ImGui::Text("Configuration");
				ImGui::Columns(1);
				
				// similar to the runrange view, the table will show a bunch of runranges for different configurations
				static std::vector<std::pair<RunRange, int>> lappd_history;
				
				// since we only show information for one LAPPD in this table,
				// expanded information about a given run can be a simpler map of row index to LAPPDConfiguration,
				// so lets lazy-load that into memory as the user expands rows, to reduce disk IO
				static std::map<int, LAPPDConfiguration> loaded_configurations;
				
				//////////////////////////////////
				// define functions to fetch data from disk
				
				static auto GetRuns = [&db](){
					
					// scan through all the runranges
					// merge entries where the configuration of our selected LAPPD does not change
					// to populate lappd_history
					static std::vector<RunRange> run_ranges = db.storage.get_all<RunRange>();
					int row_id=0;
					
					for(int i=0; i<run_ranges.size(); ++i){
						RunRange& rr = run_ranges[i];
						
						if(row_id!=0 && rr.placements.count(row_id)!=0){
							// this LAPPD configuration is still present, extend the end of the current configuration
							lappd_history.back().first.end_run = rr.end_run;
							lappd_history.back().first.end_date = rr.end_date;
						} else {
							// else see if the LAPPD of interest is in this runrange's set of placements
							row_id=0;
							for(std::pair<const int, std::string>& p : rr.placements){
								static Placement tmp_placement;
								bool ok = db.GetPlacement(p.first, tmp_placement);
								if(!ok) continue; // FIXME log error
								static LAPPD tmp_lappd;
								ok = db.GetLAPPD(tmp_placement.LAPPD_rowID, tmp_lappd);
								if(tmp_lappd.INCOM_ID==std::stoi(current_lappd)){
									// add this runrange to our LAPPD history
									lappd_history.emplace_back(std::pair<RunRange, int>(rr,p.first));
									row_id = p.first;
									break;
								}
							}
						}
					} // and loop over runranges to fill lappd_history
				}; // end GetRuns lambda
				
				// lambda to retrieve additional information about the LAPPD for a given runrange
				static auto GetLAPPD = [&db](int i, RunRange& rr){
					// make LAPPDConfiguration entry with key i
					// find this LAPPD's entry in the placements map
					LAPPDConfiguration& row_entry = loaded_configurations[i];
					bool ok = db.GetPlacement(lappd_history[i].second, row_entry.placement);
					if(!ok) goto bad_read;
					ok = db.GetLAPPD(row_entry.placement.LAPPD_rowID, row_entry.lappd);
					if(!ok) goto bad_read;
					ok = db.GetACDC(row_entry.lappd.ACDC1_rowID, row_entry.acdc1);
					if(!ok) goto bad_read;
					ok = db.GetACDC(row_entry.lappd.ACDC2_rowID, row_entry.acdc2);
					if(!ok) goto bad_read;
					row_entry.status = rr.placements[lappd_history[i].second];
					return;
					
					bad_read:
					row_entry.clear();
					return;
				};
				
				//////////////////////////////////
				// end functions to fetch data from disk
				
				// generate table data only on first run, or when selected LAPPD changes
				static std::string last_current_lappd = current_lappd;
				if(lappd_history.empty() || last_current_lappd!=current_lappd){
					lappd_history.clear();
					loaded_configurations.clear();
					GetRuns();
					last_current_lappd = current_lappd;
				}
				
				// alright now build the table
				for(int i=0; i<lappd_history.size(); ++i){
					
					ImGui::PushID(i);
					
					ImGui::Columns(6, NULL, false);
					static std::vector<int> col_widths{15, 22, 15, 22, 25, 50}; // FIXME better not to hard-code, but... needs to fit content
					for(int j=0; j<6; ++j){
						ImGui::SetColumnWidth(j, col_widths[j]);
					}
					
					ImGui::Text("%d",lappd_history[i].first.start_run);
					ImGui::NextColumn();
					ImGui::Text("%s",lappd_history[i].first.start_date.c_str());
					ImGui::NextColumn();
					ImGui::Text("%d",lappd_history[i].first.end_run);
					ImGui::NextColumn();
					ImGui::Text("%s",lappd_history[i].first.end_date.c_str());
					ImGui::NextColumn();
					ImGui::TextWrapped("%s",lappd_history[i].first.comments.c_str());
					ImGui::NextColumn();
					
					// expandable section with more info
					if (ImGui::CollapsingHeader("Configuration")){
						
						// fetch data off disk if necessary
						if(loaded_configurations.count(i)==0) GetLAPPD(i, lappd_history[i].first);
						
						ImGui::Text("Status: %s", loaded_configurations[i].status.c_str());
						ImGui::Text("Data ID: %d", loaded_configurations[i].placement.Data_ID);
						ImGui::Text("Slow Control ID: %d", loaded_configurations[i].placement.SC_ID);
						ImGui::Text("PPS Ratio: %d", loaded_configurations[i].placement.PPS_Ratio);
						ImGui::Text("Mailbox: %d", loaded_configurations[i].placement.mailbox);
						ImGui::Text("Layer: %d", loaded_configurations[i].placement.layer);
						ImGui::Text("Position: (%.2f, %.2f, %.2f)", loaded_configurations[i].placement.x, loaded_configurations[i].placement.y, loaded_configurations[i].placement.z);
						ImGui::Text("ACC: %d", loaded_configurations[i].placement.ACC_ID);
						ImGui::Text("ACDC 0: %d using pedestals %s, ACC port %d", loaded_configurations[i].acdc1.ID, loaded_configurations[i].acdc1.pedestals.c_str(), loaded_configurations[i].placement.ACDC1_ACCport1);
						ImGui::Text("ACDC 1: %d using pedestals %s, ACC port %d", loaded_configurations[i].acdc2.ID, loaded_configurations[i].acdc2.pedestals.c_str(), loaded_configurations[i].placement.ACDC1_ACCport2);
						ImGui::Text("Comments: %s", loaded_configurations[i].placement.comments.c_str());
						
					}
					
					ImGui::Columns(1);
					ImGui::PopID();
					
				} // loop over table rows
				
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem("New Entry")){
			
			if (ImGui::CollapsingHeader("Register new ACDC")){
				
				static int acdc_id = -1;
				ImGui::InputInt("ID", &acdc_id);
				static char acdc1_pedestals[255] = "";
				ImGui::InputText("Pedestals File", acdc1_pedestals, IM_ARRAYSIZE(acdc1_pedestals));
				if(ImGui::Button("Save##SaveACDC")){
					ACDC acdc1(acdc1_id, acdc1_pedestals);
					db.CreateACDC(acdc1);
					if(acdc1.rowid==0){
						std::cerr<<"error creating ACDC1 entry!"<<std::endl;
					}
				}
			}
			
			if (ImGui::CollapsingHeader("Register new LAPPD")){
				
				static int lappd_id = -1;
				ImGui::InputInt("ID", &lappd_id);
				
				static std::vector<std::string> acdcs = [&db](){
					std::vector<std::string> ids;
					for(int incom_id : db.storage.select(sqlite_orm::distinct(&LAPPD::INCOM_ID))){
						ids.push_back(std::to_string(incom_id));
					}
					return ids;
				}();
				
				// the table will show available ACDCs to assign to the new LAPPD
				
				static auto GetACDCs = [&db]() { return db.storage.get_all<ACDC>(); }
				
				static std::vector<std::pair<ACDC, int>> acdcs = GetACDCs();
				// TODO button or flag to refresh, when a new one is added
				
				// if(collabsible ACDCs){
				static std::vector<int> col_widths{5, 50, 50, 20, 20};
				ImGui::Columns(4, NULL, false);
				for(int j=0; j<4; ++j){
					ImGui::SetColumnWidth(j, col_widths[j]);
				}
				ImGui::Text("ID");
				ImGui::NextColumn();
				ImGui::Text("pedestals file");
				ImGui::NextColumn();
				ImGui::Text("comments");
				ImGui::NextColumn();
				ImGui::Text(""); // select as ACDC 0
				ImGui::NextColumn();
				ImGui::Text(""); // select as ACDC 1
				ImGui::Columns(1);
				
				static int acdc_0_rowid=-1;
				static int acdc_1_rowid=-1;
				
				for(int i=0; i<acdcs.size(); ++i){
					ImGui::PushID(i);
					ImGui::Columns(5, NULL, false);
					for(int j=0; j<5; ++j){
						ImGui::SetColumnWidth(j, col_widths[j]);
					}
					ImGui::Text("%d",acdcs[i].ID);
					ImGui::NextColumn();
					ImGui::TextWrapped("%s",acdcs[i].pedestals.c_str());
					ImGui::NextColumn();
					ImGui::TextWrapped("%d",acdcs[i].comments.c_str());
					ImGui::NextColumn();
					if(ImGui::Button("Select as ACDC 0")){ acdc_0_rowid = acdcs[i].rowid; }
					ImGui::NextColumn();
					if(ImGui::Button("Select as ACDC 0")){ acdc_1_rowid = acdcs[i].rowid; }
					ImGui::Columns(1);
				}
				
				
				static int acdc_0_index = -1;
				ImGui::Text("ACDC 0"); ImGui::SameLine();
				ImGui::SetNextItemWidth(20);
				ImGui::Combo("###acdc0_combo", &acdc_0_index, acdcs, 4);
				
				static std::string current_lappd;
				current_lappd = lappds[current_index];
				
				if(ImGui::Button("Save##SaveLAPPD")){
					LAPPD lappd1(lappd_id, acdc1.rowid, acdc2.rowid);
					db.CreateLAPPD(lappd1);
					if(lappd1.rowid==0){
						std::cerr<<"error creating LAPPD entry!"<<std::endl;
					}
				}
				
			}
			if (ImGui::CollapsingHeader("Register new Deployment")){
			
			}
			
			if (ImGui::CollapsingHeader("Register new RunRange")){
				// FIXME unlike the dummy data, we should ensure that there is no overlap
				// between the new runrange and any existing one in the database,
				// as that would result in two configurations for the same run!
			
			}
				
				ImGui::Text("LAPPDs");
//////////////////////////
				
				int lappd1_id = 73;
				
				
				int mailbox = 4;
				Layer layer = Layer::Top;
				double x = -0.111;
				double y = 0.333;
				double z = 2.444;
				int data_id = 3;
				int slowcontrol_id = 3;
				int pps_ratio = 8;
				int acc_id = 1;
				int acdc1_port1 = 2;
				int acdc1_port2 = 3;
				int acdc2_port1 = 4;
				int acdc2_port2 = 5;
				
				
				int start_run = 4228;
				int end_run = 4454;
				std::string start_date = "2023-04-27 15:38:44";
				std::string end_date = "2023-09-14 14:04:45";
				std::string comments="2023 later in the year";
				
				

				Placement placement1(mailbox, layer, x, y, z, lappd1.rowid, data_id, slowcontrol_id, pps_ratio, acc_id, acdc1_port1, acdc1_port2, acdc2_port1, acdc2_port2);
				db.CreatePlacement(placement1);
				if(placement1.rowid==0){
					std::cerr<<"error creating Placement entry!"<<std::endl;
				}
				std::map<int, std::string> placements;
				placements.emplace(placement1.rowid, "Unstable");
				RunRange runs(start_run, start_date, placements, comments, end_run, end_date);
				db.CreateRunRange(runs);
				if(runs.rowid==0){
					std::cerr<<"error creating RunRange entry!"<<std::endl;
				}


//////////////////////////
				
				//ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
				//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.7f, 0.7f));
				//ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
				if(ImGui::Button("Save")){
					run_ranges = db.storage.get_all<RunRange>();
				}
				//ImGui::PopStyleColor(3);
				
				ImGui::EndTabItem();
			}
			
			ImGui::EndTabBar();
		}
		
		ImGui::End();
		// end main window
		/////////////////
		bool demo=true;
//		ImTui::ShowDemoWindow(&demo);
//		ImGui::ShowDemoWindow(&demo);
		
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




##################################################


        const float inner_width_to_use = (flags & ImGuiTableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
        if (ImGui::BeginTable("table_advanced", 6, flags, outer_size_enabled ? outer_size_value : ImVec2(0, 0), inner_width_to_use))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            ImGui::TableSetupColumn("ID",           ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 0.0f, MyItemColumnID_ID);
            ImGui::TableSetupColumn("Name",         ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
            ImGui::TableSetupColumn("Action",       ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Action);
            ImGui::TableSetupColumn("Quantity",     ImGuiTableColumnFlags_PreferSortDescending, 0.0f, MyItemColumnID_Quantity);
            ImGui::TableSetupColumn("Description",  (flags & ImGuiTableFlags_NoHostExtendX) ? 0 : ImGuiTableColumnFlags_WidthStretch, 0.0f, MyItemColumnID_Description);
            ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

            // Sort our data if sort specs have been changed!
            ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs();
            if (sorts_specs && sorts_specs->SpecsDirty)
                items_need_sort = true;
            if (sorts_specs && items_need_sort && items.Size > 1)
            {
                MyItem::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
                qsort(&items[0], (size_t)items.Size, sizeof(items[0]), MyItem::CompareWithSortSpecs);
                MyItem::s_current_sort_specs = NULL;
                sorts_specs->SpecsDirty = false;
            }
            items_need_sort = false;

            // Take note of whether we are currently sorting based on the Quantity field,
            // we will use this to trigger sorting when we know the data of this column has been modified.
            const bool sorts_specs_using_quantity = (ImGui::TableGetColumnFlags(3) & ImGuiTableColumnFlags_IsSorted) != 0;

            // Show headers
            if (show_headers)
                ImGui::TableHeadersRow();

            // Show data
            // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
            ImGui::PushButtonRepeat(true);
#if 1
            // Demonstrate using clipper for large vertical lists
            ImGuiListClipper clipper;
            clipper.Begin(items.Size);
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
#else
            // Without clipper
            {
                for (int row_n = 0; row_n < items.Size; row_n++)
#endif
                {
                    MyItem* item = &items[row_n];
                    //if (!filter.PassFilter(item->Name))
                    //    continue;

                    const bool item_is_selected = selection.contains(item->ID);
                    ImGui::PushID(item->ID);
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height);
                    ImGui::TableNextColumn();

                    // For the demo purpose we can select among different type of items submitted in the first column
                    char label[32];
                    sprintf(label, "%04d", item->ID);
                    if (contents_type == CT_Text)
                        ImGui::TextUnformatted(label);
                    else if (contents_type == CT_Button)
                        ImGui::Button(label);
                    else if (contents_type == CT_SmallButton)
                        ImGui::SmallButton(label);
                    else if (contents_type == CT_FillButton)
                        ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f));
                    else if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow)
                    {
                        ImGuiSelectableFlags selectable_flags = (contents_type == CT_SelectableSpanRow) ? ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap : ImGuiSelectableFlags_None;
                        if (ImGui::Selectable(label, item_is_selected, selectable_flags, ImVec2(0, row_min_height)))
                        {
                            if (ImGui::GetIO().KeyCtrl)
                            {
                                if (item_is_selected)
                                    selection.find_erase_unsorted(item->ID);
                                else
                                    selection.push_back(item->ID);
                            }
                            else
                            {
                                selection.clear();
                                selection.push_back(item->ID);
                            }
                        }
                    }

                    if (ImGui::TableNextColumn())
                        ImGui::TextUnformatted(item->Name);

                    // Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
                    // and we are currently sorting on the column showing the Quantity.
                    // To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
                    // You will probably need a more advanced system in your code if you want to automatically sort when a specific entry changes.
                    if (ImGui::TableNextColumn())
                    {
                        if (ImGui::SmallButton("Chop")) { item->Quantity += 1; }
                        if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Eat")) { item->Quantity -= 1; }
                        if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                    }

                    if (ImGui::TableNextColumn())
                        ImGui::Text("%d", item->Quantity);

                    ImGui::TableNextColumn();
                    if (show_wrapped_text)
                        ImGui::TextWrapped("Lorem ipsum dolor sit amet");
                    else
                        ImGui::Text("Lorem ipsum dolor sit amet");

                    if (ImGui::TableNextColumn())
                        ImGui::Text("1234");

                    ImGui::PopID();
                }
            }
            ImGui::PopButtonRepeat();

            // Store some info to display debug details below
            table_scroll_cur = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
            table_scroll_max = ImVec2(ImGui::GetScrollMaxX(), ImGui::GetScrollMaxY());
            table_draw_list = ImGui::GetWindowDrawList();
            ImGui::EndTable();
        }
