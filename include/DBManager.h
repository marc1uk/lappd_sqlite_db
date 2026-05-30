#ifndef DBManager_H
#define DBManager_H
#include <sqlite_orm.h>
#include <iostream>
#include "schema.h"

// helper function for printouts
std::string demangle(const char* mangled_name);
bool CheckPath(std::string path, char& type);

// helper function for making storage - 100% necessary
inline auto makeStorage(const std::string &path) {
	return sqlite_orm::make_storage(path,
		sqlite_orm::make_table("RunRange",
			sqlite_orm::make_column("rowid", &RunRange::rowid, sqlite_orm::primary_key().autoincrement()),
			sqlite_orm::make_column("start_run", &RunRange::start_run),
			sqlite_orm::make_column("end_run", &RunRange::end_run),
			sqlite_orm::make_column("start_date", &RunRange::start_date),
			sqlite_orm::make_column("end_date", &RunRange::end_date),
			sqlite_orm::make_column("placements", &RunRange::setPlacements, &RunRange::getPlacements),
			sqlite_orm::make_column("comments", &RunRange::comments, sqlite_orm::default_value(""))
		),
		sqlite_orm::make_table("Placement",
			sqlite_orm::make_column("rowid", &Placement::rowid, sqlite_orm::primary_key().autoincrement()),
			sqlite_orm::make_column("mailbox", &Placement::mailbox),
			sqlite_orm::make_column("layer", &Placement::layer),
			sqlite_orm::make_column("x", &Placement::x),
			sqlite_orm::make_column("y", &Placement::y),
			sqlite_orm::make_column("z", &Placement::z),
			sqlite_orm::make_column("LAPPD_rowID", &Placement::LAPPD_rowID),
			sqlite_orm::make_column("Data_ID", &Placement::Data_ID),
			sqlite_orm::make_column("SC_ID", &Placement::SC_ID),
			sqlite_orm::make_column("PPS_Ratio", &Placement::PPS_Ratio),
			sqlite_orm::make_column("ACC_ID", &Placement::ACC_ID),
			sqlite_orm::make_column("ACDC1_ACCport1", &Placement::ACDC1_ACCport1),
			sqlite_orm::make_column("ACDC1_ACCport2", &Placement::ACDC1_ACCport2),
			sqlite_orm::make_column("ACDC2_ACCport1", &Placement::ACDC2_ACCport1),
			sqlite_orm::make_column("ACDC2_ACCport2", &Placement::ACDC2_ACCport2),
			sqlite_orm::make_column("comments", &Placement::comments, sqlite_orm::default_value(""))
		),
		sqlite_orm::make_table("LAPPD",
			sqlite_orm::make_column("rowid", &LAPPD::rowid, sqlite_orm::primary_key().autoincrement()),
			sqlite_orm::make_column("INCOM_ID", &LAPPD::INCOM_ID),
			sqlite_orm::make_column("ACDC1_rowID", &LAPPD::ACDC1_rowID),
			sqlite_orm::make_column("ACDC2_rowID", &LAPPD::ACDC2_rowID),
			sqlite_orm::make_column("comments", &LAPPD::comments, sqlite_orm::default_value(""))
		),
		sqlite_orm::make_table("ACDC",
			sqlite_orm::make_column("rowid", &ACDC::rowid, sqlite_orm::primary_key().autoincrement()),
			sqlite_orm::make_column("ID", &ACDC::ID),
			sqlite_orm::make_column("pedestals", &ACDC::pedestals),
			sqlite_orm::make_column("comments", &ACDC::comments, sqlite_orm::default_value(""))
		)
	);
}

using Storage = decltype(makeStorage(""));

class DBManager {
	public:
	DBManager(std::string db_file, bool create=false);
	
	template<typename T>
	int CreateRecord(T& new_record){
		try {
			new_record.rowid = storage.insert(new_record);
		} catch (const std::system_error& e){
			std::cout << "caught "<<e.what() <<" creating "<<demangle(typeid(T).name())<<std::endl;
			return 0;
		}
		if(m_verbose) std::cout<<"created "<<demangle(typeid(T).name())<<":\n"
		                       <<storage.dump(new_record)<<std::endl;
		return new_record.rowid;
	}
	
	int CreateACDC(ACDC&);
	int CreateLAPPD(LAPPD&);
	int CreatePlacement(Placement&);
	int CreateRunRange(RunRange&);
	
	template<typename T>
	bool GetRecord(int rowid, T& record){
		try {
			record = storage.get<T>(rowid);
			return true;
		} catch (const std::system_error& e){
			std::cout << "caught "<<e.what() <<" getting "<<demangle(typeid(T).name())
			          <<" with rowid "<<rowid<<std::endl;
		}
		return false;
	}
	
	int GetACDC(int, ACDC&);
	int GetLAPPD(int, LAPPD&);
	int GetPlacement(int, Placement&);
	int GetRunRange(int, RunRange&);
	
	bool GetRunInfo(int run_num, RunRange& runs);
	bool GetRunInfo(std::string date, RunRange& runs);
	
	void SetVerbose(int iVerbose);
	
	//private:
	int m_verbose=0;
	Storage storage;
	
};

// do i regret my choice of sqlite_orm? ... possibly.
// next time: https://github.com/srombauts/sqlitecpp

# endif
