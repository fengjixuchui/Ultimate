#ifndef __DIRENT_H__
#define __DIRENT_H__

struct dirent
{
	long d_offset;
	long d_type;
	long d_namelen;
	char d_name[];
};

struct DIR
{
	int fd;
	int buf_pos;
	int buf_end;
	char buf[256];
};

int getdents(int fd,struct dirent *buf,long count);
struct DIR* opendir(const char *path);
int closedir(struct DIR *dir);
struct dirent *readdir(struct DIR *dir);

#endif