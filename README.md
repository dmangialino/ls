# ls
C Implementation of ls command with 10 options

To compile: make myls
To run: ./myls [Directory] [-AadFilmnp1]
To clean: make clean

* The directory provided should be the fullpath name and should not end with a / 
(i.e. should be in format returned to command “pwd”)


The following options are available:

-1  Force output to be on entry per line

-A	List all entries except for . and ..

-a	Include directory entires whose names being with a dot (.) 

-d	Directories are listed as plain files (not searched recursively)

-F	Display a slash (‘/) immediately after each pathname that is a directory, an asterisk (‘*’) after each that is an
    executable, an at sign (‘@‘) after each symbolic link, an equals sign (‘=‘) after each socket, a percent sign (‘%’) after
    each whiteout, and a vertical bar (‘|’) after each that is a FIFO.	

-i	For each file, print the file’s file serial number (node number)

-l	List in long format

-m	Stream output format; list files across the page, separated by commas

-n	Display user and group IDs numerically, rather than converting to a user or group 	name in a long (-l) output

-p	Write a slash (‘/‘) after each filename if that file is a directory
