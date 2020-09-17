#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <iostream>

#include "linux_parser.h"
#include "processor.h"

using std::stoi;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
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

string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
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

float LinuxParser::MemoryUtilization() {
  string category, kilobyte_string, mem_total_string, mem_free_string;
  string line;
  int mem_total_val, mem_free_val;
  float mem_used_kb;
  float mem_utilization;

  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    for (int i = 0; i < 2; i++) {
      std::getline(stream, line);
      std::istringstream linestream(line);
      if (i == 0) {
        linestream >> category >> mem_total_string >> kilobyte_string;
        mem_total_val = std::stoi(mem_total_string);
      } else {
        linestream >> category >> mem_free_string >> kilobyte_string;
        mem_free_val = std::stoi(mem_free_string);
      }
    }
  }

  mem_used_kb = mem_total_val - mem_free_val;
  mem_utilization = mem_used_kb / mem_total_val;
  return mem_utilization;
}

float LinuxParser::CpuUtilization() {
    return 10.0;
} 

long LinuxParser::UpTime() {
  string line;
  string up_string, idle_string;
  long uptime;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    if (std::getline(filestream, line)) {
    std::istringstream linestream(line);
    linestream >> up_string >> idle_string;
    }
    uptime = lround(stod(up_string));
  }
  return uptime;
}

long LinuxParser::Jiffies() {
    string line;
    string cpu, user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    int jiffies;
    std::ifstream filestream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
    
    if (filestream.is_open()) {
        std::getline(filestream, line);
        std::istringstream linestream(line);
        linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
    }
    jiffies = stol(user) + stol(nice) + stol(system) + stol(idle) + stol(iowait) + stol(irq) + stol(softirq) + stol(steal) + stol(guest) + stol(guest_nice);
    return jiffies;
}

long LinuxParser::ActiveJiffies() {
    string line;
    string cpu, user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    int active_jifs;
    std::ifstream filestream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
    
    if (filestream.is_open()) {
        std::getline(filestream, line);
        std::istringstream linestream(line);
        linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
    }
    active_jifs = stol(user) + stol(nice) + stol(system) + stol(irq) + stol(softirq) + stol(steal);
    return active_jifs;
}

long LinuxParser::IdleJiffies() {
    string line;
    string cpu, user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    long idle_jifs;
    std::ifstream filestream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
    
    if (filestream.is_open()) {
        std::getline(filestream, line);
        std::istringstream linestream(line);
        linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
    }
    idle_jifs = stol(idle) + stol(iowait);
    return idle_jifs;
}

int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "processes") {
        return stoi(value);
      }
    }
  }
}

int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "procs_running") {
        return stoi(value);
      }
    }
  }
}

long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string value;
  std::ifstream filestream(kProcDirectory + "/" + to_string(pid) + kStatFilename);
  int user_time, kernel_time, cutime, cstime;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      for (int i = 1; i < 18; i++) {
        linestream >> value;
        if (i == 14) {
          user_time = stol(value);
        } else if (i == 15) {
          kernel_time = stol(value);
        } else if (i == 16) {
          cutime = stol(value);
        } else if (i == 17) {
          cstime = stol(value);
        }
      }
    }
  }
  return user_time + kernel_time + cutime + cstime;
}

string LinuxParser::Command(int pid) {
    string line, command;
    std::ifstream filestream(kProcDirectory + "/" + to_string(pid) + kCmdlineFilename);
    if (filestream.is_open()) {
        std::getline(filestream, line);
        std::istringstream linestream(line);
        linestream >> command;
        return command;
    }
}

float LinuxParser::CpuUtilization(int pid_) {
    string line, value, starttime;
    float cpu_utilization;
    int seconds;

    int hertz = sysconf(_SC_CLK_TCK);
    int uptime = UpTime();
    int total_time = ActiveJiffies(pid_);

    std::ifstream filestream(kProcDirectory + "/" + to_string(pid_) + kStatFilename);
    
    if (filestream.is_open()) {
        std::getline(filestream, line);
        std::istringstream linestream(line);
        for (int i = 0; i < 22; i++) {
            linestream >> starttime;
            }
        }
    
    seconds = uptime - (stol(starttime) / hertz);
    cpu_utilization = (((float) total_time / (float) hertz) / (float) seconds);

    return cpu_utilization;
}

string LinuxParser::Ram(int pid) {
    string line, key, value, ram_kb{"0"};
    int ram_mb;
    std::ifstream filestream(kProcDirectory + "/" + to_string(pid) + kStatusFilename);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::istringstream linestream(line);
            if (linestream >> key >> value) {
                if (key == "VmSize:") {
                    ram_kb = value;
                    break;
                }
            }
        }
    }
    ram_mb = stol(ram_kb) / 1000;
    return to_string(ram_mb);
}

string LinuxParser::Uid(int pid) {
    string line, key, value;
    string uid;
    std::ifstream filestream(kProcDirectory + "/" + to_string(pid) + kStatusFilename);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::istringstream linestream(line);
            linestream >> key >> value;
            if (key == "Uid:") {
                uid = value;
                return uid;
            }
        }
    }
}

string LinuxParser::User(int pid) {
    string line, key, x, value;
    string user;
    string uid = Uid(pid);
    std::ifstream filestream(kPasswordPath);
    if (filestream.is_open()) {
        while (std::getline(filestream, line)) {
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream linestream(line);
            linestream >> key >> x >> value;
            if (value == uid) {
                user = key;
                return user;
            }
        }
    }
}

long LinuxParser::UpTime(int pid) {
    string line, key;
    string starttime;
    long uptime;

    long system_uptime = UpTime();
    string uid = Uid(pid);
    int hertz = sysconf(_SC_CLK_TCK);

    std::ifstream filestream(kProcDirectory + "/" + to_string(pid) + kStatFilename);
    if (filestream.is_open()) {
        std::getline(filestream, line);
        std::istringstream linestream(line);
        for (int i=1; i < 23; i++) {
              linestream >> starttime;
        }
        uptime = (stol(starttime) / hertz);
    }
    return uptime;
}