#include <iostream>
#include <sys/types.h>
#include <sys/xattr.h>
#include <cstdio>
#include <string.h>

using namespace std;

int main(void)
{

	off_t* value= new off_t;
	string filename="user.a.c";
	*value=0x7A;    
	setxattr("./Composit.c",filename.c_str(),value,1,0);
	filename="user.b.c";
	*value=0xB8;    
	setxattr("./Composit.c",filename.c_str(),value,1,0);
	filename="user.c.c";
	*value=0x0132;    
	setxattr("./Composit.c",filename.c_str(),value,2,0);


	filename="user.a.c";
	*value=0x03FF;    
	setxattr("./SCtest/Spacecomposit.c",filename.c_str(),value,2,0);
	filename="user.b.c";
	*value=0x07FF;    
	setxattr("./SCtest/Spacecomposit.c",filename.c_str(),value,2,0);
	filename="user.c.c";
	*value=0x0BFF;    
	setxattr("./SCtest/Spacecomposit.c",filename.c_str(),value,4,0);


	filename="user.a.o";
	*value=0x07FF;    
	setxattr("./SCOtest/Spacecomposit.o",filename.c_str(),value,2,0);
	filename="user.b.o";
	*value=0x0FFF;    
	setxattr("./SCOtest/Spacecomposit.o",filename.c_str(),value,2,0);
	filename="user.c.o";
	*value=0x17FF;    
	setxattr("./SCOtest/Spacecomposit.o",filename.c_str(),value,4,0);

	delete(value);
	return 0;
}
