#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "format.h"

using std::string;

/* Get the elapsed time of the system in HH:MM:SS format */
string Format::ElapsedTime(long seconds) { 
	long hour{};
    long minute{};
    std::string elapsed_time{};
    std::ostringstream ossTime;

    /* Get hour */
    hour = seconds / 3600;

     /* Get minute */
    seconds %= 3600;
    minute = seconds / 60; 

     /* Get seconds  */
    seconds %= 60;

    /* Fill the format in the buffer using ostringstream */
    ossTime << std::setw(2) << std::setfill('0') << hour << ":" 
            << std::setw(2) << std::setfill('0') << minute << ":" 
            << std::setw(2) << std::setfill('0') << seconds;

    /* Convert it into string */
    elapsed_time = ossTime.str();
	
	return elapsed_time; 
}