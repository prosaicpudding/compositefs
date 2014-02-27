#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/xattr.h>
#include <vector>
#include <sys/types.h>

using namespace std;

off_t get_subfile_begin(string parentpath, string filename);

off_t get_subfile_end(string parentpath, string filename);

vector<string> get_subfiles(string path);

//returns the names of all directories directly in path
vector<string> get_subdirectories(string path);

string find_parent_file(string dpath, string name);

//returns the offset of the last slash in the actual path
//sets subpath and dpath to appropriate values
int resolve_path(string path, string * dpath, string * subpath);
