#include <iostream>
#include <sys/types.h>
#include <sys/xattr.h>
#include <cstdio>

//just some text cases I was using.

using namespace std;

int main (void)
{
/*	int* value=new int;
	*value=0x63;
	setxattr("./CompFile1.cmp","user.filea.txt",value,16,XATTR_REPLACE);
	*value=0x117;
	setxattr("CompFile1.cmp","user.fileb.txt",value,16,XATTR_REPLACE);
	*value=0x257;
	setxattr("CompFile1.cmp","user.filec.txt",value,16,XATTR_REPLACE);
	*value=0x2BB;
	setxattr("CompFile1.cmp","user.filed.txt",value,16,XATTR_REPLACE);


	*value=0x00;
	getxattr("./CompFile1.cmp","user.filea.txt",value,16);
	printf("%d\n",*value);

	getxattr("CompFile1.cmp","user.fileb.txt",value,16);
	printf("%d\n",*value);

	getxattr("CompFile1.cmp","user.filec.txt",value,16);
	printf("%d\n",*value);

	getxattr("CompFile1.cmp","user.filed",value,16);
	printf("%d\n",*value);


	delete(value);
*/

	char buffer[5000];
cout<<	(int)listxattr("CompFile1.cmp",buffer,5000)<<endl;
	int i=0;
	while (buffer[i])
	{
		cout<<buffer[i];
	}
	cout<<endl;
	return 0;
}
