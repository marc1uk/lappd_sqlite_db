#include <iostream>
#include "DBManager.h"

int main(){
	
	std::string db_file = "lappd_db.sqlite";
	try {
		DBManager db(db_file);
		
		// let's get the information about run 4250
		RunRange runs;
		bool ok = db.GetRunInfo(4250, runs);
		if(!ok){
			return 1; // error or no record found
		}
		
		std::cout<<"run 4250 had "<<runs.placements.size()<<" LAPPDs in the tank"<<std::endl;
		for(std::pair<const int, std::string>& entry : runs.placements){
			
			Placement placement;
			ok = db.GetPlacement(entry.first, placement);
			if(!ok) continue; // error or no record found
			
			LAPPD lappd;
			ok = db.GetLAPPD(placement.LAPPD_rowID, lappd);
			if(!ok) continue; // error or no record found
			std::cout<<"LAPPD "<<lappd.INCOM_ID<<" had status "<<entry.second<<std::endl;
			
			ACDC acdc1;
			ok = db.GetACDC(lappd.ACDC1_rowID, acdc1);
			if(!ok) continue; // error or no record found
			ACDC acdc2;
			ok = db.GetACDC(lappd.ACDC2_rowID, acdc2);
			if(!ok) continue; // error or no record found
			
			std::cout<<"It was connected to the ACDCs "<<acdc1.ID<<" and "<<acdc2.ID<<std::endl;
			std::cout<<"ACDC "<<acdc1.ID<<" used pedestals "<<acdc1.pedestals
			         <<" and was connected to ACC "<<placement.ACC_ID
			         <<" ports "<<placement.ACDC1_ACCport1<<" and "<<placement.ACDC1_ACCport2<<std::endl;
			std::cout<<"ACDC "<<acdc2.ID<<" used pedestals "<<acdc2.pedestals
			         <<" and was connected to ACC "<<placement.ACC_ID
			         <<" ports "<<placement.ACDC2_ACCport2<<" and "<<placement.ACDC2_ACCport2<<std::endl;
			std::cout<<"This LAPPD was deployed through mailbox "<<placement.mailbox
			         <<" on layer "<<LayerToString(placement.layer)
			         <<", with position ("<<placement.x<<","<<placement.y<<","<<placement.z<<")"<<std::endl;
			std::cout<<"The LAPPD was configured with data ID "<<placement.Data_ID
			         <<", slow control ID "<<placement.SC_ID
			         <<" and PPS_Ratio "<<placement.PPS_Ratio<<std::endl;
			if(!placement.comments.empty()) std::cout<<"additional notes: "<<placement.comments<<std::endl;
			
		}
		
		std::cout<<"This configuration was in place from "<<runs.start_date<<" (run "<<runs.start_run<<")";
		if(runs.end_run!=0){
			std::cout<<" until "<<runs.end_date<<" (run "<<runs.end_run<<")"<<std::endl;
		} else {
			std::cout<<" until present"<<std::endl;
		}
		
	} catch(std::exception& e){
		std::cerr<<"caught "<<e.what()<<std::endl;
		return 1;
	}
	
	return 0;
	
}
