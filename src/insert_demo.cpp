#include <iostream>
#include "DBManager.h"

int main(){
	
	std::string db_file = "lappd_db.sqlite";
	try {
		DBManager db(db_file, true); // true to create the DB
		db.SetVerbose(1); // prints information about created records
		
		int acdc1_id = 33;
		std::string acdc1_pedestals = "33.txt";
		ACDC acdc1(acdc1_id, acdc1_pedestals);
		db.CreateACDC(acdc1);
		if(acdc1.rowid==0){
			std::cerr<<"error creating ACDC1 entry!"<<std::endl;
		}
		
		int acdc2_id = 37;
		std::string acdc2_pedestals = "37.txt";
		ACDC acdc2(acdc2_id, acdc2_pedestals);
		db.CreateACDC(acdc2);
		if(acdc2.rowid==0){
			std::cerr<<"error creating ACDC2 entry!"<<std::endl;
		}
		
		int lappd1_id = 63;
		LAPPD lappd1(lappd1_id, acdc1.rowid, acdc2.rowid);
		db.CreateLAPPD(lappd1);
		if(lappd1.rowid==0){
			std::cerr<<"error creating LAPPD entry!"<<std::endl;
		}
		
		int mailbox = 2;
		Layer layer = Layer::Bottom;
		double x = -0.902975;
		double y = 0.2921;
		double z = 2.58398;
		int data_id = 2;
		int slowcontrol_id = 2;
		int pps_ratio = 5;
		int acc_id = 0;
		int acdc1_port1 = 0;
		int acdc1_port2 = 0;
		int acdc2_port1 = 0;
		int acdc2_port2 = 0;
		
		Placement placement1(mailbox, layer, x, y, z, lappd1.rowid, data_id, slowcontrol_id, pps_ratio, acc_id, acdc1_port1, acdc1_port2, acdc2_port1, acdc2_port2);
		db.CreatePlacement(placement1);
		if(placement1.rowid==0){
			std::cerr<<"error creating Placement entry!"<<std::endl;
		}
		
		int start_run = 4225;
		int end_run = 4450;
		std::string start_date = "2023-03-27 15:38:44";
		std::string end_date = "2023-07-14 14:04:45";
		std::string comments="2023";
		std::map<int, std::string> placements;
		placements.emplace(placement1.rowid, "Active");
		RunRange runs(start_run, start_date, placements, comments, end_run, end_date);
		
		db.CreateRunRange(runs);
		if(runs.rowid==0){
			std::cerr<<"error creating RunRange entry!"<<std::endl;
		}
		
	} catch(std::exception& e){
		std::cerr<<"caught "<<e.what()<<std::endl;
	}
	
	return 0;
	
}
