#ifndef __DIRENT_H__
#define __DIRENT_H__

struct dirent
{
	long d_offset;
	long d_type;
	long d_namelen;
	char d_name[];
};

#endif