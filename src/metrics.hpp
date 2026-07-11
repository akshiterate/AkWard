#pragma once //only include this header file once per compilation, allows us to skip include guard

#include <string>
#include <vector>
void increment_requests();

void increment_status(int status);

std::string metrics_to_json();
void load_metrics();
void save_metrics();

void add_bytes(size_t bytes);
std::vector<std::string> logging();
int ram_usage();
int cpu_usage();
