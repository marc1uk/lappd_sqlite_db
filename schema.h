#ifndef SCHEMA_H
#define SCHEMA_H
#include "Layer_enum.h"
#include <algorithm> // std::count
#include <stdexcept> // runtime_error

////////////////////////////////////// XXX ///////////////////////////////////////////
//               IF YOU UPDATE THE CLASS DEFINITIONS IN THIS FILE                   //
//  YOU MUST ALSO UPDATE THE DEFINTIONS IN DBManager.h makeStorage method           //
//         For each member there should be a corresponding 'make_column'            //
//   for non-simple datatypes, provide getter/setter as per RunRange::placements    //
////////////////////////////////////// XXX ///////////////////////////////////////////

struct ACDC {
	int64_t rowid = 0;
	int ID = 0;
	std::string pedestals = "";
	std::string comments = "";
	
	ACDC(int iID, std::string iPedestals, std::string iComments="") : ID(iID), pedestals(iPedestals){}
	ACDC(){}
	void Print(){
		std::cout<<"ACDC ID:"<<ID
		         <<", pedestals: "<<pedestals
		         <<", comments: '"<<comments<<"'"<<std::endl;
	}
};

struct LAPPD {
	int64_t rowid = 0;
	int INCOM_ID = 0;
	int ACDC1_rowID = 0; // top ACDC, strips 0-14
	int ACDC2_rowID = 0; // bottom ACDC, strips 15-28
	std::string comments = "";
	
	LAPPD(int iINCOMID, int iACDC1RowID, int iACDC2RowID, std::string iComments="") : INCOM_ID(iINCOMID), ACDC1_rowID(iACDC1RowID), ACDC2_rowID(iACDC2RowID), comments(iComments) {};
	LAPPD(){};
	void Print(){
		std::cout<<"LAPPD "<<INCOM_ID
		         <<", ACDC1 rowid: "<<ACDC1_rowID
		         <<", ACDC2 rowid: "<<ACDC2_rowID
		         <<", comments: '"<<comments<<"'"<<std::endl;
	}
};

struct Placement {
	int64_t rowid = 0;
	int mailbox = 0;
	Layer layer;
	double x = 0;
	double y = 0;
	double z = 0;
	int LAPPD_rowID = 0;
	int Data_ID = 0;
	int SC_ID = 0;
	int ACC_ID = 0;
	int ACDC1_ACCport1 = 0;
	int ACDC1_ACCport2 = 0;
	int ACDC2_ACCport1 = 0;
	int ACDC2_ACCport2 = 0;
	std::string comments = "";
	
	Placement(int iMailbox, Layer iLayer, double ix, double iy, double iz, int iLAPPDRowID, int iDataID, int iSCID, int iACCID, int iACDC1Port1, int iACDC1Port2, int iACDC2Port1, int iACDC2Port2, std::string iComments="") : mailbox(iMailbox), layer(iLayer), x(ix), y(iy), z(iz), LAPPD_rowID(iLAPPDRowID), Data_ID(iDataID), SC_ID(iSCID), ACC_ID(iACCID), ACDC1_ACCport1(iACDC1Port1), ACDC1_ACCport2(iACDC1Port2), ACDC2_ACCport1(iACDC2Port1), ACDC2_ACCport2(iACDC2Port2), comments(iComments) {};
	Placement(){};
	void Print(){
		std::cout<<"LAPPD rowid "<<LAPPD_rowID
		         <<", mailbox: "<<mailbox
		         <<", layer: "<<LayerToString(layer)
		         <<", (x,y,z): ("<<x<<","<<y<<","<<z<<")"
		         <<", LAPPD Data ID: "<<Data_ID
		         <<", LAPPD Slow Control ID: "<<SC_ID
		         <<", ACC ID: "<<ACC_ID
		         <<", ports "<<ACDC1_ACCport1<<","<<ACDC1_ACCport2<<","<<ACDC2_ACCport1<<","<<ACDC2_ACCport2
		         <<", comments: '"<<comments<<"'"<<std::endl;
	// or std::cout<<db.storage.dump(*this)<<std::endl; // dumps as JSON-like
	}
};

class RunRange {
	public:
	int64_t rowid = 0;
	int start_run = 0;
	int end_run = 0;
	std::string start_date = "";
	std::string end_date = "";
	std::map<int, std::string> placements; // map of Placement row IDs to status strings
	std::string comments = "";
	
	RunRange(int iStartRun, std::string iStartDate, std::map<int, std::string> iPlacements, std::string iComments, int iEndRun=0, std::string iEndDate="") : start_run(iStartRun), start_date(iStartDate), placements(iPlacements), comments(iComments), end_run(iEndRun), end_date(iEndDate){};
	RunRange(){};
	
	// json conversion stolen from Store class
	// why not use the Store class then? Annoyingly it has no way to iterate over held keys
	// plus it pulls in ToolDAQFramework and thus boost and zmq as dependencies, which is overkill
	std::string getPlacements() const {
		std::stringstream ss;
		ss<<"{";
		bool first=true;
		for (auto it=placements.begin(); it!=placements.end(); ++it){
			if (!first) ss<<",";
			ss<<"\""<<it->first<<"\":\""<< it->second<<"\"";
			first=false;
		}
		ss<<"}";
		//std::cout<<"getPlacements generated JSON "<<ss.str()<<std::endl;
		return ss.str();
	}
	
	void setPlacements(std::string input){
		if(input.length()<4) return;
		int type=0;
		std::string key;
		std::string val;
		for(size_t i = 0; i < input.length(); ++i){
			if(input[i]=='"') type++;
			else if(type==1) key+=input[i];
			else if(type==3) val+=input[i];
			else if(type==4){
				type=0;
				placements[std::stoi(key)]=val;
				key="";
				val="";
			}
		}
		//std::cout<<"setPlacements created "<<placements.size()<<" entries from inupt "<<input<<std::endl;
		return;
	}
	
};

#endif
