#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/xattr.h>

using namespace std;

string find_parent_file(string dpath, string name);

