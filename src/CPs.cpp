#include <CPs.h>
#include <CStrUtil.h>

#include <std_os.h>

#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#define COLOR_YELLOW "[33m"
#define COLOR_GREEN  "[32m"
#define COLOR_NONE   "[0m"

CPs::
CPs()
{
  user_     = getenv("USER");
  user_pid_ = getpid();

#if   defined(OS_SUN)
  ps_command_ = "ps -e -o 'user pid ppid args'";
#elif defined(OS_OSX)
  ps_command_ = "ps -e -o 'user pid ppid command'";
#elif defined(OS_LINUX)
  ps_command_ = "ps -e -o 'user pid ppid command'";
#else
  ps_command_ = "ps -elf";
#endif
}

CPs::
~CPs()
{
}

void
CPs::
loadProcesses(bool hier)
{
  root_process_ = std::make_unique<CPsRootProcess>(this);

  using ProcessList    = std::vector<CPsProcess *>;
  using ProcessMap     = std::map<int,CPsProcess *>;
  using PIdProcessList = std::map<int,ProcessList>;

  ProcessMap     processes;
  PIdProcessList pid_processes;

  FILE *fp = popen(ps_command_.c_str(), "r");

  std::string line;

  int line_num = 0;

  int c;

  while ((c = fgetc(fp)) != EOF) {
    if (c == '\n') {
      if (line_num > 0) {
        auto *process = processLine(line);

        if (process && filterProcess(process)) {
          processes[process->getPid()] = process;

          pid_processes[process->getPPid()].push_back(process);
        }
        else
          delete process;
      }

      ++line_num;

      line = "";
    }
    else
      line += char(c);
  }

  pclose(fp);

  //-------

  if (hier) {
    for (const auto &pid_process : pid_processes) {
      int         pid         = pid_process.first;
      const auto &processList = pid_process.second;

      auto p = processes.find(pid);

      CPsProcess *pprocess;

      if (p != processes.end())
        pprocess = (*p).second;
      else
        pprocess = getRootProcess();

      for (const auto &process : processList)
        pprocess->add_child(process);
    }

    for (const auto &p : processes) {
      auto *process = p.second;

      std::sort(process->child_begin(), process->child_end(), ProcessCmp1());
    }
  }
  else {
    auto *pprocess = getRootProcess();

    for (const auto &p : processes) {
      auto *process = p.second;

      pprocess->add_child(process);
    }
  }

  sort(getRootProcess()->child_begin(), getRootProcess()->child_end(), ProcessCmp1());
}

CPsProcess *
CPs::
processLine(const std::string &line)
{
  std::vector<Field> fields;

  uint i   = 0;
  uint len = line.size();

  while (i < len) {
    while (i < len && isspace(line[i]))
      ++i;

    if (i >= len)
      break;

    Field field;

    field.start = i;

    while (i < len && ! isspace(line[i]))
      field.str += line[i++];

    fields.push_back(field);
  }

  uint num_fields = fields.size();

  int         pid, ppid;
  std::string owner, command, args;

#if defined(OS_SUN)
  owner   = (num_fields > 0 ? fields[0].str : "none");
  pid     = (num_fields > 1 ? CStrUtil::toInteger(fields[1].str) : 0);
  ppid    = (num_fields > 2 ? CStrUtil::toInteger(fields[2].str) : 0);
  command = (num_fields > 3 ? line.substr(fields[3].start) : "");
  args    = "";

  i = len;
#elif defined(OS_OSX)
  owner   = (num_fields > 0 ? fields[0].str : "none");
  pid     = (num_fields > 1 ? CStrUtil::toInteger(fields[1].str) : 0);
  ppid    = (num_fields > 2 ? CStrUtil::toInteger(fields[2].str) : 0);
  command = (num_fields > 3 ? fields[3].str : "");
  args    = "";

  if (num_fields > 3)
    i = fields[3].start + fields[3].str.size();
  else
    i = len;
#elif defined(OS_LINUX)
  owner   = (num_fields > 0 ? fields[0].str : "none");
  pid     = (num_fields > 1 ? CStrUtil::toInteger(fields[1].str) : 0);
  ppid    = (num_fields > 2 ? CStrUtil::toInteger(fields[2].str) : 0);
  command = (num_fields > 3 ? fields[3].str : "");
  args    = "";

  if (num_fields > 3)
    i = fields[3].start + fields[3].str.size();
  else
    i = len;
#else
  pid     = (num_fields > 3  ? CStrUtil::toInteger(fields[3].str) : 0);
  ppid    = (num_fields > 4  ? CStrUtil::toInteger(fields[4].str) : 0);
  owner   = (num_fields > 2  ? fields[2].str : "");
  command = (num_fields > 14 ? line.substr(fields[14].start) : "");
  args    = "";

  if (num_fields > 14)
    i = fields[14].start;
  else
    i = len;
#endif

  while (i < len) {
    if (isspace(line[i])) {
      args += line[i++];

      while (i < len && isspace(line[i]))
        ++i;
    }
    else
      args += line[i++];
  }

  auto *process = new CPsProcess(pid, ppid, owner, command, args);

  return process;
}

bool
CPs::
filterProcess(CPsProcess *process)
{
  if (user_ != "") {
    if (process->getOwner() != user_)
      return false;
  }

  return true;
}

void
CPs::
printProcesses()
{
  depth_ = 0;

  if (getHtml()) {
    std::cout << "<html>\n";
    std::cout << "<head></head>\n";
    std::cout << "<body>\n";
    std::cout << "<pre>\n";
  }

  for (const auto &child : getRootProcess()->children())
    child->print();

  if (getHtml()) {
    std::cout << "</pre>\n";
    std::cout << "</body>\n";
    std::cout << "</html>\n";
  }
}

bool
CPs::
killProcess(const std::string &name, bool kill_all, bool recursive, bool force_kill)
{
  int my_pid = getpid();

  CPsProcess *process = nullptr;

  uint num_matched = 0;

  for (const auto &child : getRootProcess()->children()) {
    if (child->getPid() == my_pid) // can kill myself
      continue;

    auto name1 = child->getCommand();

    auto pos = name1.rfind('/');

    if (pos != std::string::npos)
      name1 = name1.substr(pos + 1);

    if (name1 == name) {
      process = child;

      if (kill_all) {
        if (recursive)
          child->killChildren();

        child->kill();
      }

      ++num_matched;
    }
  }

  if (kill_all)
    return true;

  if (num_matched == 0)
    return false;

  if (num_matched == 1 || force_kill) {
    if (recursive)
      process->killChildren();

    process->kill();

    return true;
  }
  else
    return false;
}

//----------

bool
CPs::ProcessCmp1::
operator()(CPsProcess *process1, CPsProcess *process2)
{
  return process1->cmpId(process2);
}

bool
CPs::ProcessCmp2::
operator()(CPsProcess *process1, CPsProcess *process2)
{
  return process1->cmpOwner(process2);
}

//----------

CPsProcess::
CPsProcess(int pid, int ppid, const std::string &owner,
           const std::string &command, const std::string &args) :
 Composite(this)
{
  data_.pid     = pid;
  data_.ppid    = ppid;
  data_.owner   = owner;
  data_.command = command;
  data_.args    = args;
}

CPsProcess::
~CPsProcess()
{
  for (auto &child : children())
    delete child;
}

void
CPsProcess::
print() const
{
  if (getPid() == getParentPs()->getUserPId())
    return;

  bool display = true;

  if (getParentPs()->getShowHead() && getParentPs()->getDepth() != 0)
    display = false;

  if (getParentPs()->getShowTail() && children_.size() != 0)
    display = false;

  if (display) {
    if (! getParentPs()->getHtml()) {
      if (! getParentPs()->getColor()) {
        if (getParentPs()->getUser() == "")
          printf("%6d %8s ", getPid(), getOwner().c_str());
        else
          printf("%6d ", getPid());
      }
      else {
        if (getParentPs()->getUser() == "")
          printf("%s%6d%s %s%8s%s ", COLOR_YELLOW, getPid(), COLOR_NONE,
                 COLOR_GREEN, getOwner().c_str(), COLOR_NONE);
        else
          printf("%s%6d%s ", COLOR_YELLOW, getPid(), COLOR_NONE);
      }

      if (! getParentPs()->getShowHead() && ! getParentPs()->getShowTail()) {
        for (int i = 0; i < getParentPs()->getDepth(); ++i)
          printf("  ");
      }

      printf("%s", getCommand().c_str());

      if (getArgs().size() > 0)
        printf(" %s", getArgs().c_str());

      printf("\n");
    }
    else {
      if (getParentPs()->getUser() == "") {
        std::cout << "<font color='blue'>";
        std::cout << getPid();
        std::cout << "<font color='black'> ";

        std::cout << "<font color='green'>";
        std::cout << getOwner();
        std::cout << "<font color='black'> ";
      }
      else {
        std::cout << "<font color='blue'>";
        std::cout << getPid();
        std::cout << "<font color='black'> ";
      }

      if (! getParentPs()->getShowHead() && ! getParentPs()->getShowTail()) {
        for (int i = 0; i < getParentPs()->getDepth(); ++i)
          std::cout << "  ";
      }

      std::cout << getCommand();

      if (getArgs().size() > 0)
        std::cout << " " << getArgs();

      std::cout << "<br>\n";
    }
  }

  getParentPs()->incDepth();

  for (const auto &child : children())
    child->print();

  getParentPs()->decDepth();
}

void
CPsProcess::
kill()
{
  ::kill(getPid(), SIGTERM);
}

void
CPsProcess::
killChildren()
{
  for (auto &child : children()) {
    child->killChildren();

    child->kill();
  }
}
