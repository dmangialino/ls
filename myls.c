/* 
	myls.c
	Main program for myls
*/

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "functions.h"

int main(int argc, char *argv[]) {
	if(argc == 1) {
		printf("Usage: ./myls [Directory (fullpath)] [-AadFilmnp1]\n");
		exit(0);
	}

	/* Get the name of the directory, requires the full pathname */
	char* filename = argv[1];

	/*	Get the selected options and populate array accordingly 
					position	option
					0		-d
					1 		-a
					2 		-A
					3 		-i
					4 		-l
					5		-1
					6 		-m
					7 		-F
					8		-p
					9 		-n
			Level of priority: -d, -a, -A
			Conflicts: -m, -1, -l, -n (the last one inputted by the user will be considered)
	*/
	int options[10];
	for(int i = 2; i < argc; ++i) {
		if(strcmp(argv[i], "-d") == 0) {
			options[0] = 1;
		} else if(strcmp(argv[i], "-a") == 0) {
			options[1] = 1;
		} else if(strcmp(argv[i], "-A") == 0) {
			options[2] = 1;
		} else if(strcmp(argv[i], "-i") == 0) {
			options[3] = 1;
		} else if(strcmp(argv[i], "-l") == 0) {
			options[4] = 1;
			/* Conflicts with -m, -1, and -n; Remove -m, -1, and -n */
			options[5] = 0;
			options[6] = 0;
			options[9] = 0;
		} else if(strcmp(argv[i], "-1") == 0) {
			options[5] = 1;
			/* Conflicts with -m and -1, Remove -m and -1 */
			options[4] = 0;
			options[6] = 0;
		} else if(strcmp(argv[i], "-m") == 0) {
			options[6] = 1;
			/* Conflicts with -l, -1 and -n; Remove -l, -1, and -n */
			options[4] = 0;
			options[5] = 0;
			options[9] = 0;
		} else if(strcmp(argv[i], "-F") == 0) {
			options[7] = 1;
		} else if(strcmp(argv[i], "-p") == 0) {
			options[8] = 1;
		} else if(strcmp(argv[i], "-n") == 0) {
			options[9] = 1;
			/* Conflicts with -l, -m, and -1; Remove -l, -m, and -1 */
			options[4] = 0;
			options[5] = 0;
			options[6] = 0;
		} else {
			printf("myls: illegal option %s\nusage: ls [Directory (fullpath)] [-AadFilmnp1]\n", argv[i]);
			exit(0);
		}
	}

	if(options[0] == 1)
		notRecursiveDir(filename, options);
	else if(options[1] == 1)
		includeHidden(filename, options);
	else if(options[2] == 1)
		includeAllButCurrPrev(filename, options);
	else
		noHiddenFiles(filename, options);

	return 0;
}





















