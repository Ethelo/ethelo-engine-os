#pragma once

#include <map>
#include <vector>
#include <string>
#include <stack>

class StopWatch{
	struct Item{
		std::string name;
		double total_time = 0.0;
		bool is_active = false;
		clock_t start_time;
		bool is_recorded = false;
	};
	
	std::map<std::string, Item> recordMap;
	std::vector<std::string> item_order; // The order for printing
	std::stack<Item*> itemStack; // Only allow one item to be active at a time to avoid double-counting
	
	//Item *BgItem= nullptr;
	std::string tag="";
	bool locked = false;
	
	void pause(Item* item);
	void resume(Item* item);
	void record_name(std::string name);
	inline double ElapsedTime(clock_t start, clock_t end);
	
  public:
	void start(std::string itemName, bool postponeList = false);
	void stop(std::string itemName);
	void stop_all();
	void clear(std::string itemName);
	void reset_all();
	
	void lock_item(std::string itemName);
	void unlock_item(std::string itemName);
	
	//std::string get_Bg_name() const;
	void setTag(std::string tag){this->tag = tag;};
	std::string getTag() const {return tag;}
	bool is_stack_empty() const { return itemStack.empty();}
	std::string topProcess() const { return (itemStack.empty()?"":itemStack.top()->name);}
	bool is_active(std::string item) const;
	
	void print(std::ostream& out);
	void print_Compact(std::ostream& out);
};

extern StopWatch globalStopWatch;