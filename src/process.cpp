#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"

using std::string;
using std::to_string;
using std::vector;

/* Return this process's ID */
int Process::Pid() { 
	return pid_;
}

/* Return this process's CPU utilization */
float Process::CpuUtilization() { 
	cpu_ = static_cast<float> (LinuxParser::ActiveJiffies(pid_)); 
	return cpu_;
}

/* Return the command that generated this process */
string Process::Command() { 
	return LinuxParser::Command(pid_);
}

/* Return this process's memory utilization */
string Process::Ram() { 
	return LinuxParser::Ram(pid_);
}

/* Return the user (name) that generated this process */
string Process::User() { 
	return LinuxParser::User(pid_); 
}

/* Return the age of this process (in seconds) */
long int Process::UpTime() { 
	return LinuxParser::UpTime(pid_); 
}

/* Overload the "less than" comparison operator for Process objects */
bool Process::operator<(Process const& a) const { 
	return this->cpu_ > a.cpu_ ? true : false;
}