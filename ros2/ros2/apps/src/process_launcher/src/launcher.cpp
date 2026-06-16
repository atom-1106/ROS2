#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "pugixml/pugixml.hpp"

// =======================
// Restart Policy
// =======================
enum RestartPolicy
{
  ALWAYS, // automatically restart the application, if kill explcitily
  ON_FAILURE, // starts the application when failed
  NEVER // never restarts the application
};

struct AppConfig
{
  std::string name;
  RestartPolicy policy;
};

// =======================
// Process Manager Class
// =======================
class ProcessManager
{
private:
  std::map<pid_t, AppConfig> pidMap;

  std::map<pid_t, long> lastProcTime;
  long lastTotalCPU = 0;

  bool running = true;
  bool is_logger_available = false;
  volatile sig_atomic_t childDied = 0;

public:
  // Constructor
  ProcessManager()
  {
    // open the logger
    // is_logger_available = true
  }

  ~ProcessManager()
  {
    // close the logger
  }

  // =======================
  // Logging
  // =======================
  void log(const std::string& msg)
  {
    std::ofstream f("manager.log", std::ios::app);

    std::string finalMsg = "[LOG] " + msg + "\n";

    if (f.is_open())
    {
      f << finalMsg;
    }
  }

  // =======================
  // CPU Functions
  // =======================
  long getTotalCPUTime()
  {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);

    std::istringstream ss(line);

    std::string cpu;
    long user, nice, system, idle;
    long iowait, irq, softirq, steal;

    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >>
      steal;

    return user + nice + system + idle + iowait + irq + softirq + steal;
  }

  long getProcessCPUTime(pid_t pid)
  {
    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(path);

    std::string line;
    std::getline(file, line);

    std::istringstream ss(line);

    std::string tmp;
    long utime, stime;

    ss >> tmp >> tmp >> tmp;

    for (int i = 0; i < 10; i++)
      ss >> tmp;

    ss >> utime >> stime;

    return utime + stime;
  }

  // =======================
  // Memory Monitoring
  // =======================
  void readStatus(pid_t pid)
  {
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream f(path);

    std::string line;

    while (std::getline(f, line))
    {
      if (line.find("VmRSS") != std::string::npos ||
          line.find("VmSize") != std::string::npos)
      {
        log("PID " + std::to_string(pid) + " -> " + line);
      }
    }
  }

  // =======================
  // Process Management
  // =======================
  pid_t startProcess(const AppConfig& app)
  {
    pid_t pid = fork();

    if (pid == 0)
    {
      execl(app.name.c_str(), app.name.c_str(), NULL);
      exit(1);
    }

    pidMap[pid] = app;

    log("Started " + app.name + " PID=" + std::to_string(pid));

    return pid;
  }

  void launchAllApps()
  {
    pugi::xml_document document;

    try
    {
      if(document.load_file("./config-example.xml"))
      {
        auto apps = document.select_node("/configuration/applications").node();
        for(auto const& app : apps.children())
        {
          std::string app_name = app.attribute("name").as_string();
          RestartPolicy policy = static_cast<RestartPolicy>(app.attribute("policy").as_int());

          std::cerr << app_name << "  : " << policy << "\n";
          AppConfig app_config{app_name, policy};
          startProcess(app_config);
          sleep(3);
        }
      }
    }
    catch(...)
    {
      std::cerr << "Error loading the file.\n";
    }
  }

  // =======================
  // Monitoring
  // =======================
  void monitorProcesses()
  {
    long totalCPU = getTotalCPUTime();

    for (auto& p : pidMap)
    {
      pid_t pid = p.first;

      if (kill(pid, 0) == 0)
      {
        long procTime = getProcessCPUTime(pid);

        if (lastProcTime.count(pid) && lastTotalCPU != 0)
        {
          long deltaProc = procTime - lastProcTime[pid];
          long deltaTotal = totalCPU - lastTotalCPU;

          int numCores = sysconf(_SC_NPROCESSORS_ONLN);

          double cpuPercent = 100.0 * deltaProc / deltaTotal * numCores;

          log("PID " + std::to_string(pid) +
              " CPU=" + std::to_string(cpuPercent) + "%");
        }

        lastProcTime[pid] = procTime;

        readStatus(pid);
      }
    }

    lastTotalCPU = totalCPU;
  }

  // =======================
  // Child Exit Handling
  // =======================
  void handleChildExit()
  {
    if (!childDied) return;

    childDied = 0;

    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
      log("Process exited PID=" + std::to_string(pid));

      AppConfig app = pidMap[pid];
      pidMap.erase(pid);

      bool shouldRestart = false;

      if (app.policy == ALWAYS)
        shouldRestart = true;
      else if (app.policy == ON_FAILURE && status != 0)
        shouldRestart = true;

      if (shouldRestart && running)
      {
        pid_t newPid = startProcess(app);
        log("Restarted PID=" + std::to_string(newPid));
      }
      else
      {
        log("Not restarting PID=" + std::to_string(pid));
      }
      sleep(1);
    }
  }

  // =======================
  // Signals (STATIC bridge)
  // =======================
  static ProcessManager* instance;

  static void sigintHandler(int)
  {
    if (instance)
    {
      instance->running = false;
      instance->log("SIGINT received, shutting down");
    }
  }

  static void sigchldHandler(int)
  {
    if (instance)
    {
      instance->childDied = 1;
    }
  }

  void setupSignals()
  {
    instance = this;

    signal(SIGINT, sigintHandler);
    signal(SIGCHLD, sigchldHandler);
  }

  // =======================
  // Loop & Cleanup
  // =======================
  void run()
  {
    setupSignals();
    launchAllApps();

    while (running)
    {
      sleep(2);

      handleChildExit();
      monitorProcesses();
    }

    cleanup();
  }

  void cleanup()
  {
    for (auto& p : pidMap)
    {
      kill(p.first, SIGTERM);
    }

    for (auto& p : pidMap)
    {
      waitpid(p.first, NULL, 0);
    }

    log("All processes terminated. Exiting.");
  }
};

// Static member definition
ProcessManager* ProcessManager::instance = nullptr;

// =======================
// MAIN
// =======================
int main()
{
  ProcessManager manager;
  manager.run();

  return 0;
}