#include <iostream>
#include <sys/types.h>
#include <sys/xattr.h>
#include <cstdio>
#include <string.h>

//just some text cases I was using.

const int BUFFER_SIZE = 5000;

using namespace std;

int main (void)
{

//	I tested on CompFile1.cmp with two extended attribute user.filea.txt and user.filesb.txt

//	int* value=new int;
//	*value=0x63;	//Do you mean the character 'c'?
//	setxattr("./CompFile1.cmp","user.filea.txt",value,16,XATTR_REPLACE);
/*	*value=0x117;	//Do you mean the character 'u'?
	setxattr("CompFile1.cmp","user.fileb.txt",value,16,XATTR_REPLACE);
	*value=0x257;	//I am not sure what this means
	setxattr("CompFile1.cmp","user.filec.txt",value,16,XATTR_REPLACE);
	*value=0x2BB;	//Same as above
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

	char buffer[BUFFER_SIZE];
	char name[20], value[20];	//Should also be re-write in const variable format to achieve better global control
	int length = listxattr("CompFile1.cmp", buffer, BUFFER_SIZE);	//Add some error check here

	//Now the buffer contains a list of attribute names, separated by a single empty character '/0'	

	cout<<"length="<<length<<endl;		//Length is the length of the attribute name list
	
	// if succeed, break it to each attribute
	int pos = 0;
	for (int i = 0; i < length; i++) {
		name[pos++] = buffer[i];
		if (buffer[i] == '\0') {
			pos = 0;		//Set to zero, so following chacters will be stored in another name, overridding
			cout<<name<<endl;	//Add getxattr function to get the value to the corresponding name
		}	
	}

	return 0;
}
