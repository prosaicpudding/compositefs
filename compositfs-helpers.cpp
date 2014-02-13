#include "compositfs-helpers.h"

//treating the xattrib values as off_t (logically), as it seemed most appropriate
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
	int fileBegin = 0;
	for (int i=0; i<subfileEnds.size(); i++)
	{//check each file end to find the one right before the selected file
		if(subfileEnds.at(i)>fileBegin && subfileEnds.at(i)<fileEnd)
			fileBegin=subfileEnds.at(i)+1;
	}	
	return fileBegin;
}

off_t get_subfile_end(string parentpath, string filename)
{

	off_t* buffer= new off_t;
	*buffer=0;
	lgetxattr(parentpath.c_str(),("user."+filename).c_str(),buffer,sizeof(off_t));
	off_t ret=*buffer;
	delete (buffer);

	return ret;

}

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

