#include "DBManager.h"
#include <cxxabi.h>
//#include <filesystem> - needs c++17 sadly
#include <sys/stat.h>

DBManager::DBManager(std::string db_file, bool create) : storage(makeStorage(db_file)){
	
	//bool exists = std::filesystem::exists(db_file);
	char type;
	bool exists = CheckPath(db_file, type);
	exists = exists && type=='f'; // only accept files
	if(!exists && !create){
		std::cerr<<"file "<<db_file<<" does not exist!"<<std::endl;
		return;
	}
	if(exists && create){
		std::cout<<db_file<<" already exists! remove it or provide a new filename to create a DB"<<std::endl;
		return;
	}
	
	if(exists){
		// simulate to ensure file matches expectation
		std::map<std::string, sqlite_orm::sync_schema_result> check = storage.sync_schema_simulate();
		for(auto&& table : check){
			if(table.second!=sqlite_orm::sync_schema_result::already_in_sync){
				std::cerr<<"schema table "<<table.first<<" on file does not match definition in memory!"<<std::endl;
				return;
			}
		}
	} else if(create){
		// create DB file according to our schema
		storage.sync_schema();
	}
	
	return;
	
}

int DBManager::CreateACDC(ACDC& acdc){
	return CreateRecord<ACDC>(acdc);
}
int DBManager::CreateLAPPD(LAPPD& lappd){
	return CreateRecord<LAPPD>(lappd);
}
int DBManager::CreatePlacement(Placement& placement){
	return CreateRecord<Placement>(placement);
}
int DBManager::CreateRunRange(RunRange& run_range){
	return CreateRecord<RunRange>(run_range);
}

int DBManager::GetACDC(int rowid, ACDC& acdc){
	return GetRecord<ACDC>(rowid, acdc);
}
int DBManager::GetLAPPD(int rowid, LAPPD& lappd){
	return GetRecord<LAPPD>(rowid, lappd);
}
int DBManager::GetPlacement(int rowid, Placement& placement){
	return GetRecord<Placement>(rowid, placement);
}
int DBManager::GetRunRange(int rowid, RunRange& run_range){
	return GetRecord<RunRange>(rowid, run_range);
}

bool DBManager::GetRunInfo(int run_num, RunRange& runs){
	try {
		std::vector<RunRange> v = storage.get_all<RunRange>(sqlite_orm::where(sqlite_orm::between(run_num, &RunRange::start_run, &RunRange::end_run)));
		if(v.size()>1){
			std::cerr<<"GetRunInfo found multiple records for run "<<run_num<<std::endl;
			return false;
		} else if(v.empty()){
			return false;
		} else {
			runs = v.front();
			return true;
		}
	} catch(std::exception& e){
		return false;
	}
}

bool DBManager::GetRunInfo(std::string date, RunRange& runs){
	try {
		std::vector<RunRange> v = storage.get_all<RunRange>(sqlite_orm::where(sqlite_orm::between(date, &RunRange::start_date, &RunRange::end_date)));
		if(v.size()){
			std::cerr<<"GetRunInfo found multiple records for date "<<date<<std::endl;
			return false;
		} else if(v.empty()){
			return false;
		} else {
			runs = v.front();
			return true;
		}
	} catch(std::exception& e){
		return false;
	}
}

void DBManager::SetVerbose(int iVerbose){
	m_verbose = iVerbose;
	return;
}

// free helper functions

std::string demangle( const char* mangled_name ){
	std::size_t len = 0;
	int status = 0;
	std::unique_ptr<char, decltype(&std::free)> ptr(__cxxabiv1::__cxa_demangle(mangled_name, nullptr, &len, &status), &std::free);
	return ptr.get();
}

bool CheckPath(std::string path, char& type){
	struct stat s;
	if(stat(path.c_str(),&s)==0){
		if(s.st_mode & S_IFDIR){        // mask to extract if it's a directory
			type='d';  //it's a directory
			return true;
		} else if(s.st_mode & S_IFREG){ // mask to check if it's a file
			type='f'; //it's a file
			return true;
		} else {
			// exists, but neither file nor directory...maybe like a device?
			type='?';
			return false;
		}
	} else {
		// does not exist - could be a pattern, e.g. "/path/to/rootfiles_*.root"
		type='\0';
		return false;
	}
	return false;
}
