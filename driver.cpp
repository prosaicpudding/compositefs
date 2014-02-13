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

	off_t* value=new off_t;
	*value=0x4F;	//address of the end of filea.txt
	setxattr("./CompFile1.cmp","user.filea.txt",value,1,0);
	*value=0x117;	//likewise
	setxattr("CompFile1.cmp","user.fileb.txt",value,2,0);
	*value=0x24F;	//etc
	setxattr("CompFile1.cmp","user.filec.txt",value,2,0);
	*value=0x2BB;	
	setxattr("CompFile1.cmp","user.filed",value,2,0); 
	//I meant for filed not to be a text file
	//it's just random binary

	*value=0x32F;
	setxattr("./CompFile2.cmp","user.file1.txt",value,2,0);

	*value=0x383;
	setxattr("./CompFile2.cmp","user.file2.txt",value,2,0);
/*
	*value=0x00;
	getxattr("./CompFile1.cmp","user.filea.txt",value,sizeof(off_t));
	printf("%li\n",*value);

	getxattr("CompFile1.cmp","user.fileb.txt",value,sizeof(off_t));
	printf("%li\n",*value);

	getxattr("CompFile1.cmp","user.filec.txt",value,sizeof(off_t));
	printf("%li\n",*value);

	getxattr("CompFile1.cmp","user.filed",value,sizeof(off_t));
	printf("%li\n",*value);
*/

	delete(value);


	char buffer[BUFFER_SIZE];
	char name[20], val[20];	//Should also be re-write in const variable format to achieve better global control
	int length = listxattr("CompFile1.cmp", buffer, BUFFER_SIZE);	//Add some error check here

	//Now the buffer contains a list of attribute names, separated by a single empty character '/0'	

	cout<<"length="<<length<<endl;		//Length is the length of the attribute name list
	
	// if succeed, break it to each attribute
/*	int pos = 0;
	for (int i = 0; i < length; i++) {
		name[pos++] = buffer[i];
		if (buffer[i] == '\0') {
			pos = 0;		//Set to zero, so following chacters will be stored in another name, overridding
			cout<<name<<endl;	//Add getxattr function to get the value to the corresponding name
		}	
	}
*/
	return 0;
}
