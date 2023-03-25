#include "zonemaps.h"
#include <algorithm>




template<typename T>
zonemap<T>::zonemap(std::vector<T> _elements, unsigned int _num_elements_per_zone, bool _sorted_flag)
{
    // constructor 
    // Your code starts here ...
    // Building the constructor
    elements = _elements;
    num_elements_per_zone = _num_elements_per_zone;
    num_zones = ceil(elements.size() / (unsigned int)num_elements_per_zone);
    sorted_flag = _sorted_flag;
    std::vector<zone<unsigned int> > zones; // Initializing the zones which is a vector of the data structure 'zone'.
    build(); // Calling the build() function to build the zonemap.
    
}

template<typename T>
int compare (const void * a, const void * b)
{
  return ( *(T*)a - *(T*)b );
}

// Build function of the zonemap
template<typename T>
void zonemap<T>::build()
{
    unsigned int index_start = 0;
    for (unsigned int i = 0; i < num_zones; i++)
    {
        unsigned int index_end = std::min(index_start + num_elements_per_zone, (unsigned int)elements.size());
        zone<T> z;
        z.elements = std::vector<T>(elements.begin() + index_start, elements.begin() + index_end); // Elements of a particular zone
        if (!(sorted_flag)){
            qsort(z.elements.data(), z.elements.size(), sizeof(T), compare<T>); // Sorting the elements of a zone
        }
        
        z.min = z.elements[0]; // Minimum value in a zone
        z.max = z.elements[z.elements.size()-1]; // Maximum value in a zone
        z.size = z.elements.size(); // Size of the zone

        zones.push_back(z); // Storing each zone in 'zones'
        index_start = index_end; // Updating the index_start
    }
}

template<typename T>
int zonemap<T>::binarySearch(std::vector<unsigned int> &input_data, unsigned int low, unsigned int high, unsigned int x){

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

// I did'nt use binary search while querying point queries because there may be multiple occurences of the key within a zone. If we don't
// have multiple occurences of key within a zone, we can use binary search.
template<typename T>
size_t zonemap<T>::query(T key)
{
    // Your code starts here ...
    size_t number = 0;
    if(!(sorted_flag)){

        for (zone z : zones){
            if ((key >= z.min) && (key <= z.max)){ // If the key value lies in a particular zone

                number += std::count(z.elements.begin(), z.elements.end(), key); // Counting the number of occurrences

            }

        }
        return number;
    }
    else{

        for (int i; i<num_zones;i++){
            bool flag = false;
            if ((key >= zones[i].min) && (key <= zones[i].max)){
                number += std::count(zones[i].elements.begin(), zones[i].elements.end(), key);
                flag = true;
            }
            if ((flag) && (i<num_zones-1) && (zones[i+1].min != key)){ // If number is in a particular zone and it is not equal to the next zone's minimum value, we break the loop.
                break;
            }

        }
        return number;

    }
}

template<typename T>
std::vector<T> zonemap<T>::query(T low, T high){

    std::vector<T> result;

    if (!(sorted_flag)){

        for(zone z:zones){
            unsigned int end;
            unsigned int count1 = 0;
            if((low >= z.min) && (low<=z.max)){ // If the low lies in a particular zone
                if (high > z.max){ // If the high lies outside the zone
                    end = z.max;
                }
                else{ // If the high lies inside the zone
                    end = high;
                }
                while (z.elements[count1]<low){ // Iterating till we reach the value of low
                    count1++;
                }
                while ((count1 < z.elements.size())&&(z.elements[count1] <=end)){ // Storing all the elements in result till we iterate until end
                    result.push_back(z.elements[count1]); 
                    count1++;
                }
            }
            else if((low<=z.min) && (high>=z.min)){ 
                if (high > z.max){
                    end = z.max;
                }
                else{
                    end = high;
                }
                while ((count1 < z.elements.size())&&(z.elements[count1] <=end)){ // Storing all the elements in result till we iterate until end
                    result.push_back(z.elements[count1]);
                    count1++;
                }
            }
        }
    }
    else{
        for(zone z:zones){
            unsigned int end;
            unsigned int count1 = 0;
            if((low >= z.min) && (low<=z.max)){ // If the low lies in a particular zone
                if (high > z.max){ // If the high lies outside the zone
                    end = z.max;
                }
                else{ // If the high lies inside the zone
                    end = high;
                }
                while (z.elements[count1]<low){ // Iterating till we reach the value of low
                    count1++;
                }
                while ((count1 < z.elements.size())&&(z.elements[count1] <=end)){ // Storing all the elements in result till we iterate until end
                    result.push_back(z.elements[count1]); 
                    count1++;
                }
            }
            else if((low<=z.min) && (high>=z.min)){ 
                if (high > z.max){
                    end = z.max;
                }
                else{
                    end = high;
                }
                while ((count1 < z.elements.size())&&(z.elements[count1] <=end)){ // Storing all the elements in result till we iterate until end
                    result.push_back(z.elements[count1]);
                    count1++;
                }
            }
            if (high < z.max){ // If the high is less than z.max, we break out of the loop
                break;
            }
        }
    }
    return result;
}

