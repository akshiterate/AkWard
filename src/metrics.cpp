#include "metrics.hpp"
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>

struct Metrics{
	unsigned int total_requests=0;
	std::uint64_t bytes_sent=0;
	unsigned int code200=0;
	unsigned int code403=0;
	unsigned int code404=0;
	unsigned int code405=0;
};
Metrics metrics;
void load_metrics(){
	std::ifstream file("../logs/metrics.dat",std::ios::binary);
	if(file)
		file.read(reinterpret_cast<char*>(&metrics),sizeof(metrics));
	file.close();
}
void save_metrics(){
	std::ofstream file("../logs/metrics.dat",std::ios::binary);
	file.write(reinterpret_cast<char*>(&metrics),sizeof(metrics));
	file.close();
}



void increment_requests(){
	metrics.total_requests++;
}
void add_bytes(size_t bytes){
	metrics.bytes_sent+=bytes;
}
void increment_status(int status){
	switch(status){
		case 200:
			metrics.code200++;
			break;
		case 403:
			metrics.code403++;
			break;
		case 404:
			metrics.code404++;
			break;
		case 405:
			metrics.code405++;
			break;
		default:
			perror("inc_status");
			break;
	}
}
unsigned long total_ram;
unsigned long avail_ram;
int ram_usage(){
	std::string label;
	unsigned long value;
	std::string unit;
	std::ifstream file("/proc/meminfo");
	while(file>>label>>value>>unit){
		if(label=="MemTotal:")
			total_ram = value;
		if(label=="MemAvailable:")
			avail_ram = value;
	}
	int used = static_cast<int>(total_ram-avail_ram);
	if(total_ram == 0) return 0;
	return static_cast<int>((used*100)/static_cast<int>(total_ram));
}
int ram_percent;

std::vector<std::string> logs;

std::vector<std::string> logging(){
	std::ifstream file("../logs/server.log");
	std::vector<std::string> temp_logs;
	std::vector<std::string> logs;
	std::string line;
	while(getline(file,line)){
		temp_logs.push_back(line);
	}
	for(int i=(temp_logs.size()-20);i<temp_logs.size();i++){
		logs.push_back(temp_logs[i]);
	}
	return logs;

}


//cpu usage
int cpu_usage(){
	std::string label;
	uint64_t user;
	uint64_t nice;
	uint64_t system;
	uint64_t idle;
	uint64_t iowait;
	uint64_t irq;
	uint64_t softirq;
	uint64_t steal;
	uint64_t guest;
	uint64_t guest_nice;
	std::ifstream file("/proc/stat");
	file>>label>>user>>nice>>system>>idle>>iowait>>irq>>softirq>>steal>>guest>>guest_nice;
	uint64_t idle_time = idle+iowait;
	uint64_t total_time = user+nice+system+idle+iowait+irq+softirq+steal;
	static uint64_t prev_total = 0;//static means its initialized once and it keeps their value the next time the function runs allowing me to get old values to compare to, to calculate cpu usage
	static uint64_t prev_idle = 0;
	uint64_t delta_total = total_time - prev_total;
	uint64_t delta_idle  = idle_time - prev_idle;
	prev_total = total_time;
	prev_idle = idle_time;
	if (prev_total == 0)
	{
		prev_total = total_time;
    		prev_idle = idle_time;
    		return 0;
	}
	uint64_t busy = delta_total - delta_idle;
	if(delta_total==0)return 0;
	int cpu_percent = static_cast<int>(busy*100/delta_total);

	return cpu_percent;
}


std::string metrics_to_json(){
	ram_percent = ram_usage();
	logs = logging();
	int cpu= cpu_usage();
	std::string json = "{\n"
		"\"total_requests\":"+std::to_string(metrics.total_requests)+",\n"
		"\"bytes_sent\":"+std::to_string(metrics.bytes_sent)+",\n"
		"\"code200\":"+std::to_string(metrics.code200)+",\n"
		"\"code403\":"+std::to_string(metrics.code403)+",\n"
		"\"code404\":"+std::to_string(metrics.code404)+",\n"
		"\"code405\":"+std::to_string(metrics.code405)+",\n"
		"\"total_ram\":"+std::to_string(total_ram)+",\n"
		"\"avail_ram\":"+std::to_string(avail_ram)+",\n"
		"\"cpu\":"+std::to_string(cpu)+",\n"
		"\"ram_percentage\":"+std::to_string(ram_percent)+",\n"
		"\"logs\":[";
	for(int i=0;i<logs.size();i++){
		json+="\"";	
		json+=logs[i];
		json+="\"";
		if(i<(logs.size()-1)) json+=",";
	}
	json+="]\n";
	json+="}";
	return json;

}
