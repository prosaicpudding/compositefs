#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/xattr.h>
#include <vector>

using namespace std;

vector<string> get_subfiles(string path);

string find_parent_file(string dpath, string name);

