#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

/* Get operating system from the filesystem */
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;

  /* get the file from the path */
  std::ifstream file_name(kOSPath);
  if (file_name.is_open()) {
    /* Get the line from the file */
    while (std::getline(file_name, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      /* Stream the line into a buffer */
      std::istringstream linestream(line);

      /* Get the OS */
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

/* Get kernel from the filesystem */
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;

  /* get the file from the path */
  std::ifstream stream(kProcDirectory + kVersionFilename);

  if (stream.is_open()) {
    /* Get the line from the file */
    std::getline(stream, line);
    /* Stream the line into a buffer */
    std::istringstream linestream(line);

    /* Get kernel from the buffer */
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

/* Get the total process ids from file */
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;

  while ((file = readdir(directory)) != nullptr) {
    /* Is this a directory */
    if (file->d_type == DT_DIR) {
      /* Is every character of the name a digit */
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }

  closedir(directory);

  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::string line{};
  std::string name{};
  std::string value{};
  double total_memory_used{}, total_memory{}, free_memory{};

  /* get the file from the path */
  std::ifstream file_name(kProcDirectory + kMeminfoFilename);

  if (file_name.is_open()) {
    /* Get the line from the file */
    while (std::getline(file_name, line)) {
      /* When : is replaced an extra space will be created, so remove the
      extra spaces before itself */
      std::remove(line.begin(), line.end(), ' ');
      std::replace(line.begin(), line.end(), ':', ' ');

      /* Stream the line into a buffer */
      std::stringstream stream(line);
      /*The tokens are broken wrt spaces */
      stream >> name >> value;

      /* Get the memory usage with respect to given keys */
      if (name == "MemTotal") {
        total_memory = std::stof(value);
      } else if (name == "MemFree") {
        free_memory = std::stof(value);
        break;
      }
    }
  }

  /* Calculate the total memory used */
  total_memory_used = total_memory - free_memory / total_memory;

  return total_memory_used;
}

/* Read and return the system uptime */
long LinuxParser::UpTime() {
    std::string up_time{}, line{};

    /* get the file from the path */
    std::ifstream file_name(kProcDirectory + kUptimeFilename);

    if (file_name.is_open()) {
        /* Get the line from the file */
        std::getline(file_name, line);
        /* Stream the line into a buffer */
        std::stringstream stream(line);

        /* Get the uptime */
        stream >> up_time;
    }

    return stol(up_time);
}

/* Read and return the total number of jiffies for the system */
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

/* Read and return the number of active jiffies for a PID */
long LinuxParser::ActiveJiffies(int pid) {
  /* CPU time spent in user mode*/
  long utime;
  /* CPU time spent in kernel mode*/
  long stime;
  /* Waited for Children CPU time spent in user mode*/
  long cutime;
  /* Waited for Children CPU time spent in kernel mode*/
  long cstime;
  std::string line{};
  std::vector<std::string> Jiffies{};

  /*Get the file from the path */
  std::ifstream file_name(kProcDirectory + std::to_string(pid) + kStatFilename);

  if (file_name.is_open()) {
    /* get the line from the file */
    std::getline(file_name, line);
    /*Stream the line into a buffer iss*/
    std::istringstream iss(line);
    /* Reads successive elements from buffer iss */
    std::istream_iterator<string> iter(iss);
    /* end of biffer */
    std::istream_iterator<string> eos;
    /* Copy the buffer elements iss to vector jiffies using iterator */
    std::copy(iter, eos, std::back_inserter(Jiffies));
    /* Now sort the string vector */
    std::sort(Jiffies.begin(), Jiffies.end());

    /* Get the active jiffies from the vector string for the process */
    utime = std::stol(Jiffies[13]);
    stime = std::stol(Jiffies[14]);
    cutime = std::stol(Jiffies[15]);
    cstime = std::stol(Jiffies[16]);
  }

  return (utime + stime + cutime + cstime);
}

/* Read and return the number of active jiffies for the system */
long LinuxParser::ActiveJiffies() {
    /* Get the total time spent by the CPU */
    vector<string> cpu_time = CpuUtilization();
    long active_jiffies{};

    /* Get the active time spent by the cpu */
    active_jiffies = (std::stol(cpu_time[CPUStates::kUser_]) +
                    std::stol(cpu_time[CPUStates::kNice_]) +
                    std::stol(cpu_time[CPUStates::kSystem_]) +
                    std::stol(cpu_time[CPUStates::kIRQ_]) +
                    std::stol(cpu_time[CPUStates::kSoftIRQ_]) +
                    std::stol(cpu_time[CPUStates::kSteal_]) +
                    std::stol(cpu_time[CPUStates::kGuest_]) +
                    std::stol(cpu_time[CPUStates::kGuestNice_]));

    return active_jiffies;
}

/* Read and return the number of idle jiffies for the system */
long LinuxParser::IdleJiffies() {
    /*Get the total time spent by the CPU */
    vector<string> cpu_time = CpuUtilization();

    /* Get the time spent idle by the cpu */
    long idle_jiffies = (std::stol(cpu_time[CPUStates::kIdle_]) +
                        std::stol(cpu_time[CPUStates::kIOwait_]));
    return idle_jiffies;
}

/* Read and return CPU utilization */
vector<string> LinuxParser::CpuUtilization() {
  std::vector<string> cpu_time{};
  std::string line;
  std::string key;
  std::string user_time, nice_time, system_time, idle_time, iowait_time,
      irq_time, softirq_time, steal_time, guest_time, guest_nice_time;

  /* Get the file from the path */
  std::ifstream file_name(kProcDirectory + kStatFilename);

  if (file_name.is_open()) {
    /* get the line from the file */
    while (std::getline(file_name, line)) {
      /*Stream the line into a buffer linestream*/
      std::istringstream linestream(line);
      while (linestream >> key >> user_time >> nice_time >> system_time >>
             idle_time >> iowait_time >> irq_time >> softirq_time >>
             steal_time >> guest_time >> guest_nice_time) {
        /* get the jiffies for "cpu" which is the aggregate value of all 
        the cpus running in the system */
        if (key == "cpu") {
          cpu_time.push_back(user_time);
          cpu_time.push_back(nice_time);
          cpu_time.push_back(system_time);
          cpu_time.push_back(idle_time);
          cpu_time.push_back(iowait_time);
          cpu_time.push_back(irq_time);
          cpu_time.push_back(softirq_time);
          cpu_time.push_back(steal_time);
          cpu_time.push_back(guest_time);
          cpu_time.push_back(guest_nice_time);
          break;
        }
      }
    }
  }

  return cpu_time;
}

/* Read and return the total number of processes */
int LinuxParser::TotalProcesses() {
  std::string line{};
  std::string key{};
  std::string value{};
  int total_processes{};

   /* Get the file from the path */
  std::ifstream file_name(kProcDirectory + kStatFilename);

  if (file_name.is_open()) {
    /* get the line from the file */
    while (std::getline(file_name, line)) {
      /*Stream the line into a buffer linestream*/
      std::istringstream linestream(line);

      /* Get the key and its value from the line */
      while (linestream >> key >> value) {
        /* If key is formatted as "processes", then total processes is
         * the value */
        if (key == "processes") {
          total_processes = stoi(value);
          break;
        }
      }
    }
  }

  return total_processes;
}

/* Read and return the number of running processes */
int LinuxParser::RunningProcesses() {
  std::string line{};
  std::string key{};
  std::string value{};
  int running_processes{};

  /* Get the file from the path */
  std::ifstream file_name(kProcDirectory + kStatFilename);

    /* get the line from the file */
    while (std::getline(file_name, line)) {
        /*Stream the line into a buffer linestream*/
        std::istringstream linestream(line);

        /* Get the key and its value from the line */
        while (linestream >> key >> value) {
          /* If key is formatted as "procs_running:", then running_processes is the value */
            if (key == "procs_running") {
              running_processes = stoi(value);
              break;
            }          
        }
    }

  return running_processes;
}

/* Read and return the command associated with a process */
std::string LinuxParser::Command(int pid) {
  std::string command;

  /* Get the file from the path */
  std::ifstream file_name(kProcDirectory + std::to_string(pid) +
                          kCmdlineFilename);

  if (file_name.is_open()) {
    /* the line from the file is the command itself */
    std::getline(file_name, command);
  }

  return command;
}

/* Read and return the memory used by a process */
string LinuxParser::Ram(int pid) {
  std::string line{};
  std::string key{};
  std::string value{};
  std::string memory{};

  /* Get the file from the path */
  std::ifstream file_name(kProcDirectory + std::to_string(pid) +
                          kStatusFilename);

  if (file_name.is_open()) {
    /* get the line from the file */
    while (std::getline(file_name, line)) {
      /*Stream the line into a buffer linestream*/
      std::istringstream linestream(line);

      /* Get the key and its value from the line */
      while (linestream >> key >> value) {
        /* If key is formatted as "VmSize:", then user id is the value 
        in terms of Kbytes */
        if (key == "VmSize:") {
          memory = std::to_string(stoi(value) / 1024);
          break;
        }
      }
    }
  }

  return memory;
}

/* Read and return the user ID associated with a process */
string LinuxParser::Uid(int pid) {
  std::string line{};
  std::string key{};
  std::string value{};
  std::string memory{};
  std::string uid{};

   /* Get the file from the path */
  std::ifstream file_name(kProcDirectory + std::to_string(pid) +
                          kStatusFilename);

  if (file_name.is_open()) {
    /* get the line from the file */
    while (std::getline(file_name, line)) {
      /*Stream the line into a buffer linestream*/
      std::istringstream linestream(line);

      /* Get the key and its value from the line */
      while (linestream >> key >> value) {
        /* If key is formatted as "Uid:", then user id is the value */
        if (key == "Uid:") {
          uid = value;
          break;
        }
      }
    }
  }

  return uid;
}

/* Read and return the user associated with a process */
std::string LinuxParser::User(int pid) {
  std::string line;
  std::string key;
  std::string str;
  std::string value;
  std::string user;

   /* Get the file from the path */
  std::ifstream file_name(kPasswordPath);

  if (file_name.is_open()) {
    /* get the line from the file */
    while (std::getline(file_name, line)) {
      /* Replace : with empty space */
      std::replace(line.begin(), line.end(), ':', ' ');
      /*Stream the line into a buffer linestream*/
      std::istringstream linestream(line);

      /* Get the value , string and the key from the line*/
      while (linestream >> value >> str >> key) {
        /* If the key matches the user id of the given pid, then
        value is the user name */
        if (key == Uid(pid)) {
          user = value;
        }
      }
    }
  }

  return user;
}

/* Read and return the uptime of a process */
long LinuxParser::UpTime(int pid) {
    long up_time;
    std::string line{};
    std::vector<std::string> Jiffies{};

    /* Get the file from the path */
    std::ifstream file_name(kProcDirectory + std::to_string(pid) + kStatFilename);

    if (file_name.is_open()) {
        /* get the line from the file */
        std::getline(file_name, line);
        /*Stream the line into a buffer iss*/
        std::istringstream iss(line);
        /* Reads successive elements from buffer iss */
        std::istream_iterator<string> iter(iss);
        /* end of biffer */
        std::istream_iterator<string> eos;
        /* Copy the buffer elements iss to vector jiffies using iterator */
        std::copy(iter, eos, std::back_inserter(Jiffies));
        /* Now sort the string vector */
        std::sort(Jiffies.begin(), Jiffies.end());

        /* Get the active jiffies from the vector string for the process */
        up_time = std::stol(Jiffies[22]);

        /* Convert the time to system clock ticks; sysconf(_SC_CLK_TCK) is 
        number of clock ticks per second in linux systems */
        up_time /= sysconf(_SC_CLK_TCK);
    }

    return up_time;
}