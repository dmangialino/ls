/* 
	functions.h
	Header file for myls functions, including implementation
*/

#ifndef FUNCTIONS
#define FUNCTIONS

#include "path_alloc.h"
#include "errHand.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <sys/xattr.h>
#include <sys/acl.h>

/* Formats date for -l option */
char* formatDate(char* str, time_t val) {
	strftime(str, 36, "%b %d %H:%M", localtime(&val));
	return str;
}

void longFormat(struct stat info, char* filename, char* fullpath, char* currFile, int F, int p, int useNum) {
	/* File type character */
	if(S_ISREG(info.st_mode)) {
		printf("-");
	} else if(S_ISDIR(info.st_mode)) {
		printf("d");
	} else if(S_ISCHR(info.st_mode)) {
		printf("c");
	} else if(S_ISBLK(info.st_mode)) {
		printf("b");
	} else if(S_ISFIFO(info.st_mode)) {
		printf("p");
	} else if(S_ISLNK(info.st_mode)) {
		printf("l");
	} else if(S_ISSOCK(info.st_mode)) {
		printf("s");
	} else {
		printf(" ");
	}

	/* Access Permissions */
	if(info.st_mode & S_IRUSR)
		printf("r");
	else
		printf("-");

	if(info.st_mode & S_IWUSR)
		printf("w");
	else
		printf("-");

	if(info.st_mode & S_IXUSR)
		printf("x");
	else
		printf("-");

	if(info.st_mode & S_IRGRP) 
		printf("r");
	else
		printf("-");

	if(info.st_mode & S_IWGRP)
		printf("w");
	else
		printf("-");

	if(info.st_mode & S_IXGRP)
		printf("x");
	else
		printf("-");

	if(info.st_mode & S_IROTH)
		printf("r");
	else
		printf("-");

	if(info.st_mode & S_IWOTH)
		printf("w");
	else
		printf("-");

	if(info.st_mode & S_IXOTH)
		printf("x");
	else
		printf("-");

	/* Check if file has extended attributes or an ACL */
	char *namebuf;
	size_t size;
	
	acl_t acl = NULL;
	acl_entry_t dummy;
	acl = acl_get_link_np(filename, ACL_TYPE_EXTENDED);
	if (acl && acl_get_entry(acl, ACL_FIRST_ENTRY, &dummy) == -1) {
		acl_free(acl);
		acl = NULL;
	}

	if(listxattr(fullpath, namebuf, size, XATTR_NOFOLLOW) != 0) {
		printf("@ ");
	} else if(acl != NULL) {
		printf("+ ");
	} else {
		printf("  ");
	}

	/* Number of links */
	printf("%d ", info.st_nlink);
	if(info.st_nlink < 10) 
		printf(" ");

	/* Owner/User */
	struct passwd *pwd;
	setpwent();
	if(useNum == 0) {
		while((pwd=getpwent())!=NULL) {
			if(pwd->pw_uid == info.st_uid)
				printf("%s ", pwd->pw_name);
		}
	} else {
		printf("%u ", info.st_uid);
	}
	endpwent();

	/* Group */
	struct group *ptrID;
	gid_t GID = info.st_gid;
	if(useNum == 0) {
		ptrID=getgrgid(GID);
		printf("%s ", ptrID->gr_name);
	} else {
		printf("%u ", info.st_gid);
	}

	/* Size of file in bytes */
	printf("%lld\t", info.st_size);
	if(info.st_size < 100 && useNum == 1) {}
	else if(info.st_size < 1000)
		printf("\t");

	/* Last modification */
	char date[36];
	formatDate(date, info.st_mtime);
	printf("%s\t", date);

	/* Name of file */
	printf("%s", currFile);
					
	/* Check for -F and -p */
	if(F == 1) {
		if(S_ISDIR(info.st_mode)) 
			printf("/\n");
		else if(info.st_mode & S_IXUSR) 
			printf("*\n");
		else if(S_ISLNK(info.st_mode))
			printf("@\n");
		else if(S_ISSOCK(info.st_mode))
			printf("=\n");
		else if(S_IFWHT == 1)
			printf("%%\n");
		else if(S_ISFIFO(info.st_mode)) 
			printf("|\n");
		else 
			printf("\n");
	} else if(p == 1) {
		if(S_ISDIR(info.st_mode)) 
			printf("/\n");
		else 
			printf("\n");
	} else 
		printf("\n");
}

void charAfterFilename(struct stat info) {
	if(S_ISDIR(info.st_mode)) 
		printf("/");
	else if(info.st_mode & S_IXUSR) 
		printf("*");
	else if(S_ISLNK(info.st_mode))
		printf("@");
	else if(S_ISSOCK(info.st_mode))
		printf("=");
	else if(S_IFWHT == 1)
		printf("%%");
	else if(S_ISFIFO(info.st_mode)) 
		printf("|");
}

/* -d option */
void notRecursiveDir(char* filename, int options[]) {
	DIR *dp;
	struct dirent *dirp;
	dp = opendir(filename);
	struct stat info;
	long unsigned numBlocks = 0;

	if(dp != NULL) {
		while((dirp = readdir(dp)) != NULL) {
			if (strcmp(dirp->d_name, ".") == 0) {
				/* Append filename + / + dirp->d_name */
				char *fullpath;
				size_t pathlen = 500;
				fullpath = path_alloc(&pathlen);
				if(pathlen <= strlen(filename)) {
					pathlen = strlen(filename) * 2;
					if((fullpath = realloc(fullpath, pathlen)) == NULL)
						err_sys("realloc failed");
				}
				strcpy(fullpath, filename);
				int n = strlen(fullpath);
				fullpath[n] = '/';
				++n;
				/* Append name of file after "/" */
				strcpy(&fullpath[n], dirp->d_name);

				if(lstat(fullpath, &info) < 0) {
					printf("Stat Error\n");
					exit(0);
				}

				/* Print inode number (-i) */
				if(options[3] == 1) { 											
					printf("%llu ", info.st_ino);
				}
				
				if(options[4] == 1 || options[9] == 1) {											
					/* List in long format */
					char* currFile = dirp->d_name;
					int F = options[7];
					int p = options[8];
					int useNum = options[9];					
					longFormat(info, filename, fullpath, currFile, F, p, useNum);
				} else {
					printf("%s", dirp->d_name);
					/* Check for -F and -p */
					if(options[7] == 1 || options[8] == 1)
						printf("/\n");
					else 
						printf("\n");
				}
			} 
		} 
	} else {
		printf("Cannot open %s\n", filename);
	}
}

/* -a option */
void includeHidden(char* filename, int options[]) {
	DIR *dp;
	struct dirent *dirp;
	dp = opendir(filename);
	struct stat info;
	int count = 0;
	long unsigned numBlocks = 0;
	bool isFirst = true;

	if(dp != NULL) {
		while((dirp = readdir(dp)) != NULL) {
			/* Append filename + / + dirp->d_name */
			char *fullpath;
			size_t pathlen = 500;
			fullpath = path_alloc(&pathlen);
			if(pathlen <= strlen(filename)) {
				pathlen = strlen(filename) * 2;
				if((fullpath = realloc(fullpath, pathlen)) == NULL)
					err_sys("realloc failed");
			}
			strcpy(fullpath, filename);
			int n = strlen(fullpath);
			fullpath[n] = '/';
			++n;
			/* Append name of file after "/" */
			strcpy(&fullpath[n], dirp->d_name);	

			if(lstat(fullpath, &info) < 0) {
				printf("Stat Error\n");
				exit(0);
			}

			/* Check if -m output method is selected */
				if(options[6] == 1) {
					if(isFirst == false)
						printf(", ");
					else
						isFirst = false;
				}

			/* Check if print inode number was selected (-i) */
			if(options[3] == 1) { 											
				printf("%llu ", info.st_ino);
			}

			if(options[4] == 1 || options[9] == 1) {											
					/* List in long format */
					char* currFile = dirp->d_name;
					int F = options[7];
					int p = options[8];
					int useNum = options[9];					
					longFormat(info, filename, fullpath, currFile, F, p, useNum);
			} else if(options[5] == 1) {
						/* Output method is -1 */
						printf("%s", dirp->d_name);

						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
							printf("\n");
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/\n");
						}	else {
							printf("\n");
						}	
					} else if(options[6] == 1) {
						/* Output method is -m */
						printf("%s", dirp->d_name);
						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/");
						}		
					} else {
						/* Default output method (columns) */
						printf("%s", dirp->d_name);

						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
							printf("\t");
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/");
						}	else {
							printf("\t");
						}
				
						if(strlen(dirp->d_name) < 8) 
							printf("\t");

						++count;
						if(count > 2) {
							count = 0;
							printf("\n");
						}
					}				
				
			
		}
		if(options[6] == 1)
			printf("\n");
	} else {
		printf("Cannot open %s\n", filename);
	}
}

/* -A option */
void includeAllButCurrPrev(char* filename, int options[]) {
	DIR *dp;
	struct dirent *dirp;
	dp = opendir(filename);
	struct stat info;
	int count = 0;
	long unsigned numBlocks = 0;
	bool isFirst = true;

	if(dp != NULL) {
		while((dirp = readdir(dp)) != NULL) {
			if(strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0) {
				/* Append filename + / + dirp->d_name */
				char *fullpath;
				size_t pathlen = 500;
				fullpath = path_alloc(&pathlen);
				if(pathlen <= strlen(filename)) {
					pathlen = strlen(filename) * 2;
					if((fullpath = realloc(fullpath, pathlen)) == NULL)
						err_sys("realloc failed");
				}
				strcpy(fullpath, filename);
				int n = strlen(fullpath);
				fullpath[n] = '/';
				++n;
				/* Append name of file after "/" */
				strcpy(&fullpath[n], dirp->d_name);	

				if(lstat(fullpath, &info) < 0) {
					printf("Stat Error\n");
					exit(0);
				}

				/* Check if -m output method is selected */
				if(options[6] == 1) {
					if(isFirst == false)
						printf(", ");
					else
						isFirst = false;
				}

				/* Print its inode number (-i) */
				if(options[3] == 1) { 											
					printf("%llu ", info.st_ino);
				}

				if(options[4] == 1 || options[9] == 1) {											
					/* List in long format */
					char* currFile = dirp->d_name;
					int F = options[7];
					int p = options[8];
					int useNum = options[9];					
					longFormat(info, filename, fullpath, currFile, F, p, useNum);
				} else if(options[5] == 1) {
						/* Output method is -1 */
						printf("%s", dirp->d_name);

						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
							printf("\n");
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/\n");
						}	else {
							printf("\n");
						}
					} else if(options[6] == 1) {
						/* Output method is -m */
						printf("%s", dirp->d_name);
						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/");
						}		
					} else {
						/* Default output method (columns) */
						printf("%s", dirp->d_name);

						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
							printf("\t");
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/");
						}	else {
							printf("\t");
						}
				
						if(strlen(dirp->d_name) < 8) 
							printf("\t");

						++count;
						if(count > 2) {
							count = 0;
							printf("\n");
						}
					}				
				}
			
		}
		if(options[6] == 1)
			printf("\n");
	} else {
		printf("Cannot open %s\n", filename);
	}
}

/* Default (i.e. did not select -d, -a, or -A */
void noHiddenFiles(char* filename, int options[]) {
	DIR *dp;
	struct dirent *dirp;
	dp = opendir(filename);
	struct stat info;
	int count = 0;
	long unsigned numBlocks = 0;
	bool isFirst = true;

	if(dp != NULL) {
		while((dirp = readdir(dp)) != NULL) {
			if(strncmp(dirp->d_name, ".", 1) != 0) {
				/* Append filename + / + dirp->d_name */
				char *fullpath;
				size_t pathlen = 500;
				fullpath = path_alloc(&pathlen);
				if(pathlen <= strlen(filename)) {
					pathlen = strlen(filename) * 2;
					if((fullpath = realloc(fullpath, pathlen)) == NULL)
						err_sys("realloc failed");
				}
				strcpy(fullpath, filename);
				int n = strlen(fullpath);
				fullpath[n] = '/';
				++n;
				/* Append name of file after "/" */
				strcpy(&fullpath[n], dirp->d_name);	

				if(lstat(fullpath, &info) < 0) {
					printf("Stat Error\n");
					exit(0);
				}

				/* Check if -m output method is selected */
				if(options[6] == 1) {
					if(isFirst == false)
						printf(", ");
					else
						isFirst = false;
				}

				/* Print inode number (-i) */
				if(options[3] == 1) { 											
					printf("%llu ", info.st_ino);
				}

				if(options[4] == 1 || options[9] == 1) {											
					/* List in long format */
					char* currFile = dirp->d_name;
					int F = options[7];
					int p = options[8];
					int useNum = options[9];					
					longFormat(info, filename, fullpath, currFile, F, p, useNum);
				} else if(options[5] == 1) {
						/* Output method is -1 */
						printf("%s", dirp->d_name);

						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
							printf("\n");
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/\n");
						}	else {
							printf("\n");
						}	
					} else if(options[6] == 1) {
						/* Output method is -m */
						printf("%s", dirp->d_name);
						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/");
						}		
					} else {
						/* Default output method (columns) */
						printf("%s", dirp->d_name);

						/* Check if -F or -p (respectively) was selected */
						if(options[7] == 1) {			
							charAfterFilename(info);
							printf("\t");
						} else if(options[8] == 1)  {					
							if(S_ISDIR(info.st_mode)) 
								printf("/");
						}	else {
							printf("\t");
						}
				
						if(strlen(dirp->d_name) < 8) 
							printf("\t");

						++count;
						if(count > 2) {
							count = 0;
							printf("\n");
						}
					}				
				}
			
		}
		if(options[6] == 1)
			printf("\n");
	} else {
		printf("Cannot open %s\n", filename);
	}
}

#endif















