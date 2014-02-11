#include "compositfs-helpers.h"

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
                char* none;
                int n=0;
                int xattribsize=llistxattr (thepath.c_str(),none,(size_t)0);
                char xattribs [xattribsize];

                if (xattribsize!=-1)
                {
                        llistxattr(thepath.c_str(),xattribs,(size_t)xattribsize);
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
                                        if (nextfile==name)
                                        {
                                                parentfound==1;
                                                break;
                                        }
                                }
                                else continue;
                        }
                }
                else continue;
                if (parentfound==1)
                        break;
        }

        if (parentfound==1)
                return de->d_name;
        else return "";
}

