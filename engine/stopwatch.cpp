#include "stopwatch.hpp"

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <stdexcept>
#include <assert.h>

using namespace std;

StopWatch globalStopWatch;

inline double StopWatch::ElapsedTime(clock_t start, clock_t end){
	return ((double)( end - start )) / CLOCKS_PER_SEC;
}

void StopWatch::pause(StopWatch::Item* item){
	assert(item->is_active);
	
	item-> is_active = false;
	item->total_time += ElapsedTime(item->start_time, clock());
}

void StopWatch::resume(StopWatch::Item* item){
	assert(!(item->is_active));
	
	item->is_active = true;
	item->start_time = clock();
}

void StopWatch::record_name(string name){
	assert(!(recordMap.at(name).is_recorded));
	
	recordMap.at(name).is_recorded = true;
	item_order.push_back(name);
}

// General Items
void StopWatch::start(string itemName, bool postponeList){
	if (locked){	return; }
	if (recordMap.find(itemName) == recordMap.end()){
		recordMap[itemName].name = itemName;
		if (!postponeList){
			record_name(itemName);
		}
	}
	
	if (recordMap[itemName].is_active){
		throw runtime_error{"StopWatch: Starting ["+itemName+"] when it's already active"};
	}
	
	if (!itemStack.empty()){
		pause(itemStack.top());
	}
	itemStack.push(&(recordMap[itemName]));
	resume(&(recordMap[itemName]));
}

void StopWatch::stop(string itemName){	
	if (locked){	return; }
	if (recordMap.find(itemName) == recordMap.end()){
		throw runtime_error("StopWatch: Attempting to stop timer" + itemName+", which does not exist");
	}

	if (!recordMap.at(itemName).is_active){
		throw runtime_error("StopWatch: Attempting to stop timer" + itemName + ", which is already inactive");
	}
	assert(itemStack.top()->name == itemName);
	
	pause(itemStack.top());
	if (!(itemStack.top()->is_recorded)){
		record_name(itemStack.top()->name);
	}
	itemStack.pop();
	
	if (!itemStack.empty()){ resume(itemStack.top());}
}

void StopWatch::stop_all(){
	if (locked){	return; }
	while(!itemStack.empty()){
		stop(itemStack.top()->name);
	}
}

void StopWatch::clear(string itemName){
	if (locked){	return; }
	if (recordMap.find(itemName) == recordMap.end()){
		return;
	}
	
	StopWatch::Item& item = recordMap.at(itemName);
	item.is_active = false;
	item.total_time = 0.0;
}

void StopWatch::reset_all(){
	recordMap.clear();
	item_order.clear();
}

bool StopWatch::is_active(string item) const{
	if (recordMap.find(item) == recordMap.end()){
		return false;
	}
	return recordMap.at(item).is_active;
	
}

void StopWatch::print(ostream& out){
	out << "Timer Info (Cumulative) :\n";
	for (auto& itemName : item_order){
		out << "\t" << itemName << "\t:\t" << recordMap.at(itemName).total_time;
		out << (recordMap.at(itemName).is_active? "\t+\n": "\n");
	}
	out << "---End of Timer Info.---\n";
}

void StopWatch::print_Compact(ostream& out){
	out << "Timer Info (Cumulative, Compact) :\n";
	ostringstream sout1, sout2;
	
	for (auto& itemName : item_order){
		sout1 << "\t" << itemName;
		sout2 << "\t" << recordMap.at(itemName).total_time << (recordMap.at(itemName).is_active? "*": "");
	}
	out << sout1.str() << "\n";
	out << sout2.str() << "\n";
	out << "---End of Compact Timer Info.---\n";
}

void StopWatch::lock_item(std::string itemName){
	assert( (! locked ) && itemName == itemStack.top()->name);
	locked = true;
}
void StopWatch::unlock_item(std::string itemName){
	assert(locked && itemName == itemStack.top()->name);
	locked = true;
}