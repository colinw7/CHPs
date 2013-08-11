#ifndef CPS_H
#define CPS_H

#include <CComposite.h>
#include <CChild.h>
#include <CAutoPtr.h>
#include <string>

class CPs;

struct CPsProcessData {
  int         pid;
  int         ppid;
  std::string owner;
  std::string command;
  std::string args;
};

class CPsProcess : public CComposite<CPsProcess, CPsProcess>,
                   public CChild<CPsProcess> {
 private:
  typedef CComposite<CPsProcess, CPsProcess> Composite;
  typedef CChild<CPsProcess>                 ProcessChild;

  CPsProcessData data_;

 public:
  CPsProcess(int pid, int ppid, const std::string &owner,
             const std::string &command, const std::string &args) :
   Composite(this) {
    data_.pid     = pid;
    data_.ppid    = ppid;
    data_.owner   = owner;
    data_.command = command;
    data_.args    = args;
  }

  virtual ~CPsProcess() { }

  int getPid () const { return data_.pid ; }
  int getPPid() const { return data_.ppid; }

  const std::string &getOwner() const { return data_.owner; }
  const std::string &getCommand() const { return data_.command; }
  const std::string &getArgs() const { return data_.args; }

  CPsProcess *getParentProcess() const { return ProcessChild::getParent(); }

  virtual CPs *getParentPs() const { return getParent()->getParentPs(); }

  bool cmpId(CPsProcess *process) {
    return (data_.pid < process->data_.pid);
  }

  bool cmpOwner(CPsProcess *process) {
    if (getParentProcess() != NULL && process->getParentProcess() != NULL)
      return getParentProcess()->cmpId(process->getParentProcess());

    if (getParentProcess() != NULL)
      return false;

    if (process->getParentProcess() != NULL)
      return true;

    if (data_.owner != process->data_.owner)
      return (data_.owner < process->data_.owner);

    return cmpId(process);
  }

  void print() const;

  void kill();

  void killChildren();
};

class CPsRootProcess : public CPsProcess {
 private:
  CPs *ps_;

 public:
  CPsRootProcess(CPs *ps) :
   CPsProcess(0, 0, "root", "", ""), ps_(ps) {
  }

  CPs *getParentPs() const { return ps_; }
};

class CPs {
 private:
  class ProcessCmp1 {
   public:
    bool operator()(CPsProcess *process1, CPsProcess *process2);
  };

  class ProcessCmp2 {
   public:
    bool operator()(CPsProcess *process1, CPsProcess *process2);
  };

  struct Field {
    int         start;
    std::string str;
  };

 public:
  CPs();

  const std::string &getUser() const { return user_; }

  int getUserPId() const { return user_pid_; }

  int getDepth() const { return depth_; }

  bool getShowHead() const { return show_head_; }
  bool getShowTail() const { return show_tail_; }

  bool getColor() const { return color_; }

  void incDepth() { ++depth_; }
  void decDepth() { --depth_; }

  void setUser(const std::string &user) { user_ = user; }

  void setShowHead(bool show) { show_head_ = show; }
  void setShowTail(bool show) { show_tail_ = show; }

  void setColor(bool color) { color_ = color; }

  CPsRootProcess *getRootProcess() const { return root_process_; }

  void loadProcesses(bool hier=false);

  void printProcesses();

  bool killProcess(const std::string &name, bool kill_all=false,
                   bool recursive=false, bool force_kill=false);

 private:
  CPsProcess *processLine(const std::string &line);

  bool filterProcess(CPsProcess *process);

 private:
  std::string              user_;
  int                      user_pid_;
  int                      depth_;
  bool                     show_head_;
  bool                     show_tail_;
  bool                     color_;
  CAutoPtr<CPsRootProcess> root_process_;
  std::string              ps_command_;
};

#endif
