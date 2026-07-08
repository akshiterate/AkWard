#include "metrics.hpp"

int total_requests = 0;

void increment_requests(){
	total_requests++;
}
int get_total_requests(){
	return total_requests;
}
