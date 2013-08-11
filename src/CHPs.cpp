#include <CPs.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#define USAGE "\
CHPs [-me] [-user <user>] [-a|-all] [-head] [-tail] [-h|-help]\n\
\n\
-a|-all      : Display processes for all users\n\
-me          : Display processes for current user (default)\n\
-user <user> : Display processes for user <user>\n\
-head        : Show head\n\
-tail        : Show tail\n\
-nocolor     : No Color output\n\
-h|-help     : Display usage\n\
"

int
main(int argc, char **argv)
{
  CPs ps;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if      (strcmp(&argv[i][1], "me") == 0) {
        ps.setUser(getenv("USER"));
      }
      else if (strcmp(&argv[i][1], "user") == 0) {
        if (i < argc - 1)
          ps.setUser(argv[++i]);
        else
          fprintf(stderr, "Missing value for %s\n", argv[i]);
      }
      else if (strcmp(&argv[i][1], "a"  ) == 0 ||
               strcmp(&argv[i][1], "all") == 0) {
        ps.setUser("");
      }
      else if (strcmp(&argv[i][1], "head") == 0) {
        ps.setShowHead(true);
      }
      else if (strcmp(&argv[i][1], "tail") == 0) {
        ps.setShowTail(true);
      }
      else if (strcmp(&argv[i][1], "nocolor") == 0) {
        ps.setColor(false);
      }
      else if (strcmp(&argv[i][1], "h"   ) == 0 ||
               strcmp(&argv[i][1], "help") == 0) {
        fprintf(stderr, USAGE);
        exit(1);
      }
      else
        fprintf(stderr, "Invalid option %s\n", argv[i]);
    }
    else {
      fprintf(stderr, "Invalid argument %s\n", argv[i]);
    }
  }

  ps.loadProcesses(/*hier=*/true);

  ps.printProcesses();

  return 0;
}
