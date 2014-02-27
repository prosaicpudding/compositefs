#include "compositfs-helpers.h"
//**Note**
//treating the xattrib values as off_t (logically), as it seemed most appropriate

//**Note2**
//any negative xattrib value will cause the entry name to be interpreted as a directory
// any / in an attrib name will be interpreted as a path separator

//***
// This function will return the beginning offset for the file 
// 'filename' which is a subfile of 'parentpath.'
// 'parentpath' should be an absolute path. 
// February 2014
// Helen
//***
off_t get_subfile_begin(string parentpath, string filename)
{
	vector <string> subfiles = get_subfiles(parentpath);
	vector <off_t> subfileEnds;
	int fileindex=-1; //the index of the parameter file
	for (int i=0; i<subfiles.size(); i++)
	{//get the ends of each file
	//this is necessary, because xattribs are unordered (as far as I could tell)
		subfileEnds.push_back( get_subfile_end(parentpath, subfiles.at(i)) );
		if(filename==subfiles.at(i))
			fileindex=i;
	}	
	if (fileindex==-1)
	{//this was a bad call
	//the subfile isn't under the given parent
		return 0;
	}
	int fileEnd = subfileEnds.at(fileindex);
	if(fileEnd<0)
		return 0;
	int fileBegin = 0;
	for (int i=0; i<subfileEnds.size(); i++)
	{//check each file end to find the one right before the selected file
		if(subfileEnds.at(i)>fileBegin && subfileEnds.at(i)<fileEnd)
			fileBegin=subfileEnds.at(i)+1;
	}	
	return fileBegin;
}

//***
// This function will return the end offset for the file 
// 'filename' which is a subfile of 'parentpath.'
// 'parentpath' should be an absolute path. 
// February 2014
// Helen
//***
off_t get_subfile_end(string parentpath, string filename)
{

	off_t* buffer= new off_t;
	*buffer=0;
	lgetxattr(parentpath.c_str(),("user."+filename).c_str(),buffer,sizeof(off_t));
	off_t ret=*buffer;
	if (ret<0)
		return 0;
	delete (buffer);

	return ret;

}

//***
// This function will return a vector containing the names of 
// all subfiles in the file 'path'.
// 'path' should be the absolute path to a file.
// February 2014
// Helen
//***
vector<string> get_subfiles(string path)
{
	vector <string> ret;
	char* none;
        int n=0;
        int xattribsize=llistxattr (path.c_str(),none,(size_t)0);
        char xattribs [xattribsize];
        if (xattribsize!=-1)
        {
                llistxattr(path.c_str(),xattribs,(size_t)xattribsize);
                int i=0;
                while(i<xattribsize)
                {
                        string nextfile;
                        while(xattribs[i]!='\0')
                        {
                                nextfile+=xattribs[i];
                                ++i;
                        }
                        ++i;
                        if(nextfile.at(0)=='u'&&nextfile.at(1)=='s'&&nextfile.at(2)=='e'&&
                        nextfile.at(3)=='r'&&nextfile.at(4)=='.')
                        {
                                nextfile=nextfile.substr(5);
                                ret.push_back(nextfile);
                         }
                }
        }
	return ret;
}


//***
// This function will return the name of the parent file 
// for the file 'name' in directory 'dpath.'
// 'dpath' should the absolute path to the directory of the file.
// February 2014
// Helen
//***
string find_parent_file(string dpath, string name)
{
        DIR *dp;
        struct dirent *de;
        dp = opendir(dpath.c_str());
        if (dp == NULL)
                return "";
        bool parentfound=0;
        while ((de = readdir(dp)) != NULL)
        {
                string thepath=dpath+"/"+de->d_name;

		vector<string> subfiles=get_subfiles(thepath);

		for (int i=0; i<subfiles.size(); i++)
		{
			if (subfiles.at(i)==name)
				parentfound=1;
		}

                if (parentfound==1)
                        break;
	}

        if (parentfound==1)
                return de->d_name;
        else return "";
}

int resolve_path(string path, string * dpath=NULL, string * subpath=NULL)
{
	int nextslash;
	nextslash=path.find_first_of("/")+1;
	string nextpath=path.substr(0,nextslash);
	int canaccess=access(nextpath.c_str(),F_OK);
	while (canaccess==0)
	{
		nextslash=path.find_first_of("/",nextslash)+1;
		nextpath=path.substr(0,nextslash);
		canaccess=access(nextpath.c_str(),F_OK);
	}

	if (nextslash==0)
		nextslash=path.length();
	
	if(dpath!=NULL)
		*dpath=path.substr(0,nextslash);
	if(subpath!=NULL)
		*subpath=path.substr(nextslash);
	return nextslash;
}
