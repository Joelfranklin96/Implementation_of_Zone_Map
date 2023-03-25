#include <fstream>
#include <ctime>
#include <algorithm>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "args.hxx"
#include <cmath>


// Specify your path of workload file here
std::string kInputDataPath = "./workload.dat";
std::string kPointQueriesPath = "./point_queries.txt";
std::string kRangeQueriesPath = "./range_queries.txt";

// All the workload parameters
struct Parameters {
	int N; // the number of total elements to load
	uint32_t UB; //  the upper bound of the largest positive element. uint32_t - Unsigned. Range of values from 0 to (2^32) - 1
	int P; // the number of total point queries. Point query is when we look for a particular value of an attribute.
	int R; // the number of total range queries. Range query is when look for a range of values of an attribute.
	double s; // the selectivity for each range query
	bool sort_flag; // the flag that indicates if the input workload is sorted
};

int parse_arguments(int argc, char *argv[], Parameters & params);
void generate_input_data(std::string & output_path, Parameters & params,
		std::vector<int> & input_data);
void generate_point_queries(std::string & output_path, Parameters & params,
		std::vector<int> & /*input_data*/);
void generate_range_queries(std::string & output_path, Parameters & params,
		std::vector<int> & input_data);
int binarySearch(std::vector<int> & input_data, int low, int high, int x);

int main(int argc, char *argv[]) { // argc is argument count and argv is argument vector
	Parameters params;             // argc will be the number of strings pointed to by argv. This will (in practice) be 1 plus the number of arguments, as virtually all implementations will prepend the name of the program to the array.
	if (parse_arguments(argc, argv, params)) { // 0 is used to represent false and 1 is used to represent true
		exit(1); // exit(0) is used to indicate exit success and exit(1) is used to indicate exit failure
	}
	std::vector<int> data;
	generate_input_data(kInputDataPath, params, data);
	generate_point_queries(kPointQueriesPath, params, data);
	generate_range_queries(kRangeQueriesPath, params, data);

	return 0;
}

int parse_arguments(int argc, char *argv[], Parameters & params) {
	args::ArgumentParser parser("Zonemap_Workload_Parser.", "");
	args::Group group(parser, "Workload parameters:", args::Group::Validators::DontCare);
	args::ValueFlag<int> number_of_total_elements_cmd(group, "N",
			"The number of total elements  [def: 1000000]", {'N', "num-points"});
	args::ValueFlag<uint32_t> upper_bound_of_largest_element_cmd(group, "UB",
			"The upper bound of the largest positive element  [def: 3000000]",
			{"UB", "upper-bound-of-the-max-element"});
	args::ValueFlag<int> number_of_point_queries_cmd(group, "P",
			"The number of total point queries [def: 10000] (duplicates may exist "
			"in the input data, there might be more than one matching records)",
			{'P', "num-point-queries"});
	args::ValueFlag<int> number_of_range_queries_cmd(group, "R",
			"The number of total range queries [def: 1000]", {'R', "num-range-queries"});
	args::ValueFlag<double> selectivity_of_range_queries_cmd(group, "s",
			"The selectivity for each range queries (0 <= s <= 1), [def: 0.5]",
			{'s', "selectivity-of-range-queries"});
        args::Flag sorted_cmd(group, "is_sorted", "sort the input workload", {"sort"});

	try {
		parser.ParseCLI(argc, argv);
	}
	catch (args::Help&) {
		std::cout << parser;
		exit(0);
	}
	catch (args::ParseError& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	catch (args::ValidationError& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}

	params.N =  number_of_total_elements_cmd ? args::get(number_of_total_elements_cmd) : 1000000;
	params.P = number_of_point_queries_cmd ? args::get(number_of_point_queries_cmd) : 10000;
	params.R = number_of_range_queries_cmd ? args::get(number_of_range_queries_cmd) : 1000;
	params.UB = upper_bound_of_largest_element_cmd ?
		args::get(upper_bound_of_largest_element_cmd) : 3000000;
	params.s = selectivity_of_range_queries_cmd ?
		args::get(selectivity_of_range_queries_cmd) : 0.5;
	params.sort_flag = sorted_cmd ? args::get(sorted_cmd) : false;

	//params.N =  100;
	//params.P = 10;
	//params.R = 10;
	//params.UB = 10000;
	//params.s = 0.5;
	//params.sort_flag = false;
	return 0;
}


void generate_input_data(std::string & output_path, Parameters & params,
		std::vector<int> & input_data) {
	input_data.clear();
	input_data.resize(params.N);
	// construct a trivial random generator engine from a time-based seed:
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();	
	std::default_random_engine gen (seed);
	std::uniform_int_distribution<int>  dist(0, params.UB); // Produces random integer values i uniformly distributed in the closed
	// interval [a,b]
	std::ofstream output_file(output_path, std::ios::binary);
	if (params.sort_flag){
		for (size_t i = 0; i < params.N; i++) {
			input_data[i] = dist(gen);
		}
		sort(input_data.begin(), input_data.end());
		for (size_t i = 0; i < params.N; i++) {
        		output_file.write(reinterpret_cast<char *>(&input_data[i]), sizeof(int));
		}
	} else {
		for (size_t i = 0; i < params.N; i++) {
			input_data[i] = dist(gen);
        		output_file.write(reinterpret_cast<char *>(&input_data[i]), sizeof(int));
		}
	}
	output_file.close();
}

void generate_point_queries(std::string & output_path, Parameters & params, 
		std::vector<int> & input_data) {

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();	
	std::default_random_engine gen (seed);
	std::uniform_int_distribution<int>  dist1(0, (size_t) (2.0*params.UB));
	std::uniform_int_distribution<int>  dist2(0, input_data.size() - 1);
	std::ofstream output_file(output_path);
	srand(time(NULL));
	for (size_t i = 0; i < params.P; i++) {
		if (rand()*1.0/RAND_MAX <= 0.2) {
			// with 0.2 probability, randomly generate point queries, may contain existing
			// or non-exising queries
			output_file << dist1(gen) << std::endl;
		} else {
			// generate existing queries
			output_file << input_data[dist2(gen)] << std::endl;
		}
	}
	// The above process may produce duplicate point queries. And the number of exsiting quries
	// has 0.8*P as the lower bound.
	output_file.close();

}
void generate_range_queries(std::string & output_path, Parameters & params,
		std::vector<int> & input_data) {
	// Your code starts here ...

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();	
	std::default_random_engine gen (seed);
	std::uniform_int_distribution<int>  dist1(0, (size_t) (params.UB));
	std::uniform_int_distribution<int>  dist2(0, input_data.size() - 1);
	std::ofstream output_file(output_path);
	srand(time(NULL));

	int total_elements = floor(params.N * params.s);
	int length = input_data.size();

	if (!(params.sort_flag)){ // If the input data is not sorted, we sort it
		sort(input_data.begin(), input_data.end());
	}

	for (size_t i = 0; i < params.R; i++){ // For loop for generating range queries

		if (rand()*1.0/RAND_MAX <= 0.2){ // if condition for 20% probability

			int lower_bound = dist1(gen); // Generating lower_bound random number in range between [0, UB]
			int index1 = binarySearch(input_data, 0, length-1, lower_bound);
			int count1 = 0;

			if (index1 != -1){ // if lower_bound number is present in input_data
                output_file << input_data[index1] << " ";
				// We keep iterating count1 till we the number of elements in the range = total_elements
				while ((count1 < total_elements-1) & ((index1+count1) < length-1)) {
					count1++;
				}
                output_file << input_data[index1+count1];
			}
			else{ // if lower_bound is not present in input_data
				int index2 = 0; // We iterate index2 till input_data[indexs2] is just greater than the lower_bound
				while ((index2<length) & (input_data[index2] < lower_bound)){
					index2++;
				}
                output_file << input_data[index2] << " ";
				int count2 = 0; // We keep iterating count2 till we the number of elements in the range = total_elements
				while ((count2 < total_elements-1) & ((index2+count2) < length-1)) {
					count2++;
				}
                output_file << input_data[index2+count2];
			}
	    }
		else{
			int lower_bound = input_data[dist2(gen)]; // Generating lower_bound random number in range between [0, N]
			int index = binarySearch(input_data, 0, length-1, lower_bound);
            output_file << input_data[index] << " ";
			int count = 0; // We keep iterating count2 till we the number of elements in the range = total_elements
			while ((count < total_elements-1) & ((index+count) < length-1)) {
                count++;
			}
            output_file << input_data[index+count];

		}
		output_file << std::endl;
	
	}
	output_file.close();
}

// Implementating binary search

int binarySearch(std::vector<int> & input_data, int low, int high, int x)
{
	if (low>high){
		return -1;
	}
	else{
		int mid = (int) (low+high)/2;		

		if (input_data[mid] == x){
			return mid;
		}
		else if(input_data[mid] > x){
			high = mid - 1;
			return binarySearch(input_data, low, high , x);
		}
		else{
			low = mid + 1;
			return binarySearch(input_data, low, high , x);
		}
	}
	
}



