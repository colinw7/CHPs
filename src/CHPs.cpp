#include <CPs.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#define USAGE "\
CHPs [-me] [-user <user>] [-a|-all] [-head] [-tail] [-html] [-h|-help]\n\
\n\
-a|-all       : Display processes for all users\n\
-me           : Display processes for current user (default)\n\
-user <user>  : Display processes for user <user>\n\
-head         : Show head\n\
-tail         : Show tail\n\
-nocolor      : No Color output\n\
-match <text> : text to match\n\
-html         : HTML output format\n\
-h|-help      : Display usage\
"

int
main(int argc, char **argv)
{
  CPs ps;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      std::string arg = &argv[i][1];

      if      (arg == "me") {
        ps.setUser(getenv("USER"));
      }
      else if (arg == "user") {
        if (i < argc - 1)
          ps.setUser(argv[++i]);
        else
          std::cerr << "Missing value for " << argv[i] << "\n";
      }
      else if (arg == "a" || arg == "all") {
        ps.setUser("");
      }
      else if (arg == "head") {
        ps.setShowHead(true);
      }
      else if (arg == "tail") {
        ps.setShowTail(true);
      }
      else if (arg == "nocolor") {
        ps.setColor(false);
      }
      else if (arg == "match") {
        if (i < argc - 1)
          ps.setMatch(argv[++i]);
        else
          std::cerr << "Missing value for " << argv[i] << "\n";
      }
      else if (arg == "nomatch") {
        if (i < argc - 1)
          ps.setNoMatch(argv[++i]);
        else
          std::cerr << "Missing value for " << argv[i] << "\n";
      }
      else if (arg == "html") {
        ps.setHtml(true);
      }
      else if (arg == "h" || arg == "help") {
        std::cerr << USAGE << "\n";
        exit(1);
      }
      else
        std::cerr << "Invalid option " << argv[i] << "\n";
    }
    else {
      std::cerr << "Invalid argument " << argv[i] << "\n";
    }
  }

  ps.loadProcesses(/*hier=*/true);

  ps.printProcesses();

  return 0;
}
