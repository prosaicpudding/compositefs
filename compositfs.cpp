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


static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;

/* something in here is causing the errors*/
// ls: cannot access /mnt: Software caused connection abort
// bash: cd: /mnt: Transport endpoint is not connected
/* working on this issue -h*/


//	string dpath=path;
//	size_t lastslash=dpath.find_last_of("/");
//	dpath=dpath.substr(0,lastslash);
//	string filename = dpath.substr(lastslash+1);
		


	if(res = lstat(path, stbuf) == 0)
		return 0; //not a composit file

	//find which file the comp file is in
//	string parentfile=find_parent_file(dpath,filename);
//	if (parentfile=="")
//		return -errno;
//	res=lstat((dpath+"/"+parentfile).c_str(),stbuf);
	//set stbuf->st_size to the actual size


	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


//#ifdef HAVE_SETXATTR
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
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
		thepath+="/";
		thepath+=de->d_name;
		
		char* none;
		int n=0;
		int xattribsize=llistxattr (thepath.c_str(),none,(size_t)0);
		char xattribs [xattribsize];

//		if (llistxattr(thepath.c_str(),xattribs,(size_t)xattribsize)!=-1)
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
				if (filler(buf, nextfile.c_str(), &st, 0))
				{	
//?					return -errno;
					break;
				}
			}
			if(iscmp==0)
			filler(buf, de->d_name, &st, 0);
		} 
		else
		{
			if (filler(buf, de->d_name, &st, 0))
			{	
//?				return -errno;
				break;
			}
		}
	}

	closedir(dp);
	return 0;
}
//#endif

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_UTIMENSAT
static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

/*working
string cmp_path_to_file(const char* path)
{//should only be called is the path does not lead 
//to a valid regular file (indicating it is to a cmpfile
	string pathstring=path;
	size_t lastSlash
	if (lastSlash=pathstring.find_last_of('/')>=pathstring.length())
		pathstring="./"
	else pathstring=pathstring.substr(0,lastSlash+1);
	if (access(path)!=0)
		return "";
	//more	
}
*/
static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
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
static int xmp_fallocate(const char *path, int mode,
			off_t offset, off_t length, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	if (mode)
		return -EOPNOTSUPP;

	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = -posix_fallocate(fd, offset, length);

	close(fd);
	return res;
}
#endif

//#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	//The size of size should always be 16 for now
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
//#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper;
//	xmp_oper.init		=xmp_init;
//	xmp_oper.destroy	=xmp_destroy;
/*	xmp_oper.getattr	= xmp_getattr;
	xmp_oper.access		= xmp_access;
	xmp_oper.readlink	= xmp_readlink;
	xmp_oper.readdir	= xmp_readdir;
	xmp_oper.mknod		= xmp_mknod;
	xmp_oper.mkdir		= xmp_mkdir;
	xmp_oper.symlink	= xmp_symlink;
	xmp_oper.unlink		= xmp_unlink;
	xmp_oper.rmdir		= xmp_rmdir;
	xmp_oper.rename		= xmp_rename;
	xmp_oper.link		= xmp_link;
	xmp_oper.chmod		= xmp_chmod;
	xmp_oper.chown		= xmp_chown;
	xmp_oper.truncate	= xmp_truncate;
#ifdef HAVE_UTIMENSAT
	xmp_oper.utimens	= xmp_utimens;
#endif
	xmp_oper.open		= xmp_open;
	xmp_oper.read		= xmp_read;
	xmp_oper.write		= xmp_write;
	xmp_oper.statfs		= xmp_statfs;
	xmp_oper.release	= xmp_release;
	xmp_oper.fsync		= xmp_fsync;
#ifdef HAVE_POSIX_FALLOCATE
	xmp_oper.fallocate	= xmp_fallocate;
#endif
#ifdef HAVE_SETXATTR
	xmp_oper.setxattr	= xmp_setxattr;
	xmp_oper.getxattr	= xmp_getxattr;
	xmp_oper.listxattr	= xmp_listxattr;
	xmp_oper.removexattr	= xmp_removexattr;
#endif
*/

void xmp_operations() {
//      xmp_oper.init           =xmp_init;
//      xmp_oper.destroy        =xmp_destroy;
        xmp_oper.getattr        = xmp_getattr;
        xmp_oper.access         = xmp_access;
        xmp_oper.readlink       = xmp_readlink;
        xmp_oper.readdir        = xmp_readdir;
        xmp_oper.mknod          = xmp_mknod;
        xmp_oper.mkdir          = xmp_mkdir;
        xmp_oper.symlink        = xmp_symlink;
        xmp_oper.unlink         = xmp_unlink;
        xmp_oper.rmdir          = xmp_rmdir;
        xmp_oper.rename         = xmp_rename;
        xmp_oper.link           = xmp_link;
        xmp_oper.chmod          = xmp_chmod;
        xmp_oper.chown          = xmp_chown;
        xmp_oper.truncate       = xmp_truncate;
#ifdef HAVE_UTIMENSAT
        xmp_oper.utimens        = xmp_utimens;
#endif
        xmp_oper.open           = xmp_open;
        xmp_oper.read           = xmp_read;
        xmp_oper.write          = xmp_write;
        xmp_oper.statfs         = xmp_statfs;
        xmp_oper.release        = xmp_release;
        xmp_oper.fsync          = xmp_fsync;
#ifdef HAVE_POSIX_FALLOCATE
        xmp_oper.fallocate      = xmp_fallocate;
#endif
#ifdef HAVE_SETXATTR
        xmp_oper.setxattr       = xmp_setxattr;
        xmp_oper.getxattr       = xmp_getxattr;
        xmp_oper.listxattr      = xmp_listxattr;
        xmp_oper.removexattr    = xmp_removexattr;
#endif
}

void usage(char* name) {
	printf("Usage\n");	//Will be detailed later
}

int main(int argc, char *argv[])
{
	xmp_operations();

	if (argc < 2) {
		usage(argv[0]);
		return 0;
	}
	
	basepath = argv[1];	//Use this basepath in almost every operation, cncatinate the basepath and the path passed in to get the absolute path. An example is written in readdir

/* This is a just simple way to do it. To make a fully functional version of arguments parseing approach, even with optional arguments, you can use fuse_opt_parse and struct Opt to do it. For now, it is not necessary*/

	umask(0);
	return fuse_main(argc-1, argv+1, &xmp_oper, NULL);
}
