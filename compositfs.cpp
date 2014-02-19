/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall fusexmp.c `pkg-config fuse --cflags --libs` -o fusexmp
*/

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include "compositfs-helpers.h"
// determine the system's max path length
#ifdef PATH_MAX
    const int pathmax = PATH_MAX;
#else
    const int pathmax = 1024;
#endif

#include <string>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
//#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
//#endif

const char *basepath;

using namespace std;


static int cmp_getattr(const char *path, struct stat *stbuf)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	if(res = lstat(name, stbuf) == 0)
		return 0; //not a composit file

	string dpath=name;
	size_t lastslash=dpath.find_last_of("/");
	while(lastslash==dpath.length()-1)
	{
		dpath=dpath.substr(0,lastslash-1);
		lastslash=dpath.find_last_of("/");
	}

	string filename = dpath.substr(lastslash+1);
	dpath=dpath.substr(0,lastslash);

	//find which file the comp file is in
	string parentfile=find_parent_file(dpath,filename);
	if (parentfile=="")
		return -errno;
	res=lstat((dpath+"/"+parentfile).c_str(),stbuf);

	//set stbuf->st_size to the actual size
	off_t begin=get_subfile_begin(dpath+"/"+parentfile,filename);
	off_t end=get_subfile_end(dpath+"/"+parentfile,filename);
	stbuf->st_size=(end-begin)+1; //offset +1, because addresses start from 0

	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_access(const char *path, int mask)
{
	int res;
	
	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = access(name, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_readlink(const char *path, char *buf, size_t size)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = readlink(name, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


//#ifdef HAVE_SETXATTR
static int cmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;	
	struct dirent *de;

	(void) offset;
	(void) fi;


	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	dp = opendir(name);

	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) 
	{//check each entry in the directory
		struct stat st;
		memset(&st, 0, sizeof(st));
		//allocate space for the stat struct
		st.st_ino = de->d_ino;

	//this was in the example version
		st.st_mode = de->d_type << 12;
	//but I don't get the shift. Is it right?

		string thepath=name;
		//will be the path to each directory entry
		thepath+="/"; //fixed 
		thepath+=de->d_name;
		
		char* none;
		int n=0;
		int xattribsize=llistxattr (thepath.c_str(),none,(size_t)0);
		char xattribs [xattribsize];

		if(xattribsize!=-1)
		{//for each xatrtib which is a sub file, add it to the buffer
			llistxattr(thepath.c_str(),xattribs,(size_t)xattribsize);
			
			int i=0;
			bool iscmp=0;
			//checks to make sure the xattribs aren't just system stuff
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
					iscmp=1;
					nextfile=nextfile.substr(5);
				}
				else
					continue;
				if (filler(buf, nextfile.c_str(), &st, 0)==-1)
				{	
					return -errno;
//					break;
				}
			}
			if(iscmp==0)
			filler(buf, de->d_name, &st, 0);
		} 
		else
		{
			if (filler(buf, de->d_name, &st, 0)==-1)
			{	
				return -errno;
//				break;
			}
		}
	}
	
	closedir(dp);
	return 0;
}
//#endif

static int cmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	if (S_ISREG(mode)) {
		res = open(name, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(name, mode);
	else
		res = mknod(name, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_mkdir(const char *path, mode_t mode)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = mkdir(name, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_unlink(const char *path)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = unlink(name);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_rmdir(const char *path)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = rmdir(name);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_chmod(const char *path, mode_t mode)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = chmod(name, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = lchown(name, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_truncate(const char *path, off_t size)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = truncate(name, size);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_UTIMENSAT
static int cmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, name, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int cmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	if(get_subfiles(name).size()!=0)
	{//ignore this file, it's a composit parent file
		return -errno;
	}
	res = open(name, fi->flags);
	if (res < 0)
	{//the file is either composit or non-existant
		string dpath=name;
		size_t lastslash=dpath.find_last_of("/");
		while(lastslash==dpath.length()-1)
		{
			dpath=dpath.substr(0,lastslash-1);
			lastslash=dpath.find_last_of("/");
		}

		string filename = dpath.substr(lastslash+1);
		dpath=dpath.substr(0,lastslash);
		string parentfile= find_parent_file(dpath,filename);
		string parentpath= dpath +"/"+ parentfile;
		
		//try to open as compositfile if there is one
		if(parentfile!="")//if the parent file was found
			res = open (parentpath.c_str(), fi->flags);

		if (res<0)//if the parent path could not be opened
			  //or the file didn't exist
	
			return -errno;
	}	

	close(res);
	return 0;
}

static int cmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	string parentfile="";
	int begin=0, end=0;
	string parentpath;

	if(get_subfiles(name).size()!=0)
	{//ignore this file, it's a composit parent file
		return -errno;
	}

	fd = open(name, O_RDONLY);
	if (fd < 0)
	{//file is either composit of non existant	
		//check if file is composit
		string dpath=name;
		size_t lastslash=dpath.find_last_of("/");
		while(lastslash==dpath.length()-1)
		{//this shouldn't have to be called but...
			dpath=dpath.substr(0,lastslash-1);
			lastslash=dpath.find_last_of("/");
		}

		string filename = dpath.substr(lastslash+1);
		dpath=dpath.substr(0,lastslash);
		parentfile = find_parent_file(dpath,filename);
		string parentpath= dpath +"/"+ parentfile;
		
		if (parentfile=="") //if there is no parent file, 
				    //subfile does not exist
			return -errno;

		//if so get begin and end
		begin=get_subfile_begin(parentpath, filename);
		end=get_subfile_end(parentpath, filename);

		//if out of range, handle appropriately
		if (offset<begin)
			offset=begin;
		if(size>end-begin+1)
			size=end-begin+1;

		//open the parentfile
		fd = open(parentpath.c_str(), O_RDONLY);
	}
	res = pread(fd, buf, size, offset);
	if (res < 0)
		res = -errno;

	close(fd);
	return res;
}

static int cmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	(void) fi;
	fd = open(name, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int cmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	res = statvfs(name, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int cmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int cmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int cmp_fallocate(const char *path, int mode,
			off_t offset, off_t length, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	char name[pathmax], *ptr;
	strncpy(name, basepath, sizeof(name));
	strncat(name, path, sizeof(name)-strlen(name));

	if (mode)
		return -EOPNOTSUPP;

	fd = open(name, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = -posix_fallocate(fd, offset, length);

	close(fd);
	return res;
}
#endif

//#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int cmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	char pname[pathmax], *ptr;
	strncpy(pname, basepath, sizeof(pname));
	strncat(pname, path, sizeof(pname)-strlen(pname));

	int res = lsetxattr(pname, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int cmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	char pname[pathmax], *ptr;
	strncpy(pname, basepath, sizeof(pname));
	strncat(pname, path, sizeof(pname)-strlen(pname));

	//The size of size should always be 16 for now
	int res = lgetxattr(pname, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int cmp_listxattr(const char *path, char *list, size_t size)
{
	char pname[pathmax], *ptr;
	strncpy(pname, basepath, sizeof(pname));
	strncat(pname, path, sizeof(pname)-strlen(pname));

	int res = llistxattr(pname, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int cmp_removexattr(const char *path, const char *name)
{
	char pname[pathmax], *ptr;
	strncpy(pname, basepath, sizeof(pname));
	strncat(pname, path, sizeof(pname)-strlen(pname));

	int res = lremovexattr(pname, name);
	if (res == -1)
		return -errno;
	return 0;
}
//#endif /* HAVE_SETXATTR */

static struct fuse_operations cmp_oper;
//	cmp_oper.init		=cmp_init;
//	cmp_oper.destroy	=cmp_destroy;
/*	cmp_oper.getattr	= cmp_getattr;
	cmp_oper.access		= cmp_access;
	cmp_oper.readlink	= cmp_readlink;
	cmp_oper.readdir	= cmp_readdir;
	cmp_oper.mknod		= cmp_mknod;
	cmp_oper.mkdir		= cmp_mkdir;
	cmp_oper.symlink	= cmp_symlink;
	cmp_oper.unlink		= cmp_unlink;
	cmp_oper.rmdir		= cmp_rmdir;
	cmp_oper.rename		= cmp_rename;
	cmp_oper.link		= cmp_link;
	cmp_oper.chmod		= cmp_chmod;
	cmp_oper.chown		= cmp_chown;
	cmp_oper.truncate	= cmp_truncate;
#ifdef HAVE_UTIMENSAT
	cmp_oper.utimens	= cmp_utimens;
#endif
	cmp_oper.open		= cmp_open;
	cmp_oper.read		= cmp_read;
	cmp_oper.write		= cmp_write;
	cmp_oper.statfs		= cmp_statfs;
	cmp_oper.release	= cmp_release;
	cmp_oper.fsync		= cmp_fsync;
#ifdef HAVE_POSIX_FALLOCATE
	cmp_oper.fallocate	= cmp_fallocate;
#endif
#ifdef HAVE_SETXATTR
	cmp_oper.setxattr	= cmp_setxattr;
	cmp_oper.getxattr	= cmp_getxattr;
	cmp_oper.listxattr	= cmp_listxattr;
	cmp_oper.removexattr	= cmp_removexattr;
#endif
*/

void cmp_operations() {
//      cmp_oper.init           =cmp_init;
//      cmp_oper.destroy        =cmp_destroy;
        cmp_oper.getattr        = cmp_getattr;
        cmp_oper.access         = cmp_access;
        cmp_oper.readlink       = cmp_readlink;
        cmp_oper.readdir        = cmp_readdir;
        cmp_oper.mknod          = cmp_mknod;
        cmp_oper.mkdir          = cmp_mkdir;
        cmp_oper.symlink        = cmp_symlink;
        cmp_oper.unlink         = cmp_unlink;
        cmp_oper.rmdir          = cmp_rmdir;
        cmp_oper.rename         = cmp_rename;
        cmp_oper.link           = cmp_link;
        cmp_oper.chmod          = cmp_chmod;
        cmp_oper.chown          = cmp_chown;
        cmp_oper.truncate       = cmp_truncate;
#ifdef HAVE_UTIMENSAT
        cmp_oper.utimens        = cmp_utimens;
#endif
        cmp_oper.open           = cmp_open;
        cmp_oper.read           = cmp_read;
        cmp_oper.write          = cmp_write;
        cmp_oper.statfs         = cmp_statfs;
        cmp_oper.release        = cmp_release;
        cmp_oper.fsync          = cmp_fsync;
#ifdef HAVE_POSIX_FALLOCATE
        cmp_oper.fallocate      = cmp_fallocate;
#endif
#ifdef HAVE_SETXATTR
        cmp_oper.setxattr       = cmp_setxattr;
        cmp_oper.getxattr       = cmp_getxattr;
        cmp_oper.listxattr      = cmp_listxattr;
        cmp_oper.removexattr    = cmp_removexattr;
#endif
//        cmp_oper.fstat    = cmp_fstat;
}

void usage(char* name) {
	printf("Usage\n");	//Will be detailed later
}

int main(int argc, char *argv[])
{
	cmp_operations();

	if (argc < 2) {
		usage(argv[0]);
		return 0;
	}
	
	basepath = argv[1];	//Use this basepath in almost every operation, cncatinate the basepath and the path passed in to get the absolute path. An example is written in readdir

/* This is a just simple way to do it. To make a fully functional version of arguments parseing approach, even with optional arguments, you can use fuse_opt_parse and struct Opt to do it. For now, it is not necessary*/

	umask(0);
	return fuse_main(argc-1, argv+1, &cmp_oper, NULL);
}
