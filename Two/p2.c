/* 
Thomas Martinson
Program 2
Professor Carroll
CS570
Due: 10/10/18

p2.c acts as a simple command line argument interpreter for a UNIX system.
This file will issue a prompt for the user and read the input and do stuff with it.
*/

#include "getword.h"
#include "p2.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#define MAXWORDS 100
extern int amp_flag;

typedef enum {
	false, true
} bool;

char *newargv[MAXWORDS];	//argument array
char s[MAXWORDS*STORAGE];	//storage array
char *input_file;
char *output_file;
int flag_in;	//0 indicates no input flag
int flag_out;	//0 indicates no output flag
int flag_pipeline;	//0 indicates no pipeline flag
int pipeIndex;

void myHandler(int signum);

int parse();

int main(int argc, char *argv[]){
	int num_arg;	//number of args returned by parse()
	int input_fd;	//input file descriptor
	int output_fd;	//output file descriptor
	pid_t childpid;
	int cd_correct;
	
	(void) signal(SIGTERM,myHandler);
	setpgid(0,0);	//set group id in order to separate p4 from running shell

	while(true){
		flag_in = 0;
		flag_out = 0;
		flag_pipeline = 0;
		pipeIndex = 0;
		fflush(stdin);
		amp_flag = 0;
		
		printf(":570: ");	//give prompt
		num_arg = parse();
		
		if(num_arg == 255){	//return 255 if EOF is reached
			break;
		}

		if(num_arg == 0){	//no command line argument, give prompt again
			continue;
		}

		if(num_arg == -2){
			fprintf(stderr, "Ambiguous output redirect. \n");
			fflush(stderr);
			continue;
		}

		if(num_arg == -3){	//Means more than one pipeline
			fprintf(stderr, "Multiple pipelines not allowed. \n");
			fflush(stderr);
			continue;
		}

		if(strcmp(newargv[0],"cd")==0){	//handle built-in cd command
			if(num_arg > 2){	//incorrect cd call
				fprintf(stderr,"chdir: too many args\n");
				fflush(stderr);
				continue;
			}

			if(newargv[1] == NULL){	//no parameters -> cd $HOME
				cd_correct = chdir(getenv("HOME"));
				if(cd_correct != 0){	//fail to go home
					perror("chdir: cannot go back to HOME.");
					continue;
				}
			}
			else{	//cd to given path
				cd_correct = chdir(newargv[1]);
				if(cd_correct != 0){ //fail to go to given path
					perror("path name error.");
					continue;
				}
				continue;
			}
		}

		//
		if(flag_pipeline != 0){ //handling pipelines
			fflush(stdout);
			fflush(stderr);
			childpid = fork();
			
			if(childpid<0){	//if fork fails
				perror("child fork failed");
				continue;
			}

			if(childpid == 0){	//code that child will execute
				pid_t grandchild_pid;
				int mypipefd[flag_pipeline*2];
				int return_value;
				return_value = pipe(mypipefd);

				if(return_value == -1){
					perror("Pipeline error");
					exit(8);
				}

				grandchild_pid = fork();
				if(grandchild_pid < 0){
					perror("grandchild fork failed");
					continue;
				}

				if(grandchild_pid == 0){
					dup2(mypipefd[1],STDOUT_FILENO);	//stdout points to write side of pipe
					close(mypipefd[0]);
					close(mypipefd[1]);

					if(flag_in == 1){	//grandchild only able to redirect input not output
						input_fd = open(input_file, O_RDONLY);	//open input file
						dup2(input_fd, STDIN_FILENO);	//redirect stdin to file
						close(input_fd);	//close input file descriptor
					}

					if(execvp(newargv[0],newargv)<0){	//execute the file, exit if fail
						perror(newargv[0]);
						_exit(10);
					}
				}	//end of grandchild code

				if(amp_flag == 1){	//background job redirects stdin to dev/null
					int dev_null = open("/dev/null",O_WRONLY);	//open dev/null fd
					dup2(dev_null,STDERR_FILENO);
				}

				if(flag_out == 1){	//if child has output redirect, will not have input redirect because it reads from mypipefd[0]
					mode_t mode = S_IRWXU | S_IRWXG | S_IROTH;
					if((output_fd = open(output_file, O_WRONLY | O_CREAT | O_EXCL, mode))<0){
						perror(output_file);
						_exit(4);
					}
					dup2(output_fd,STDOUT_FILENO);	//redirect stdout to file
					close(output_fd);	//close output_fd file descriptor
				}

				dup2(mypipefd[0],STDIN_FILENO);
				close(mypipefd[0]);
				close(mypipefd[1]);

				if(execvp(newargv[pipeIndex+1],newargv+pipeIndex+1)<0){
					perror(newargv[0]);	//if execution failed, print error
					_exit(9);
				}
			}	//end of child code block
			
			if(amp_flag == 1){//if the child's job is background, parent does not wait
				printf("%s [%d]",newargv[pipeIndex+1],childpid);
				continue;
			}

			while(true){//parent waits for child to finish
				pid_t pid;
				pid = wait(NULL);

				if(pid==childpid){
					break;
				}
			}
			continue;

			
		}

		//handle normal situations
		fflush(stdout);
		fflush(stderr);
		childpid = fork();

		if(childpid < 0){//fork failure
			fprintf(stderr,"fork failed. \n");
			fflush(stderr);
			continue;
		}

		if(childpid == 0){//fork a child to do jobs
			if(amp_flag == 1){//this is a background job
				int dev_null = open("/dev/null",O_WRONLY);
				dup2(dev_null,STDERR_FILENO);
			}
			
			if(flag_in == 1){//handle input redirect
				if((input_fd = open(input_file,O_RDONLY))<0){//open the input file
					perror(input_file);
					_exit(1);
				}

				if(dup2(input_fd,STDIN_FILENO)<0){//redirect the stdin to file
					perror(input_file);
					_exit(2);
				}

				if(close(input_fd)<0){//close the input_fd
					perror(input_file);
					_exit(3);
				}
			}//end input redirection handling

			if(flag_out == 1){//handle output redirection
				mode_t mode = S_IRWXU | S_IRWG | S_IROTH;
				if((output_fd = open(output_file, O_WRONLY | O_CREAT | O_EXCL, mode))<0){
					perror(output_file);
					_exit(4);
				}

				if(dup2(output_fd,STDERR_FILENO)<0){
					perror(output_file);
					_exit(5);
				}

				if(close(output_fd)<0){//close output fd
					perror(output_file);
					_exit(6);
				}

			}//end handling output redirection

			if(execvp(newargv[0],newargv)<0){//child executes cmd
				perror(newargv[0]);
				_exit(7);
			}

			_exit(0);
		
		}//end of child's job

		if(amp_flag == 1){//if background job, parent reissues prompt
			printf("%s [%d]\n",newargv[0],childpid);
			continue;
		}

		while(true){//parent waits for child to finish
			pid_t pid;
			pid = wait(NULL);

			if(pid == childpid){
				break;
			}

		}



	}
	killpg(getpg(getpgrp(),SIGTERM));
	printf("p2 terminated.\n");
	exit(0);

}

int parse(){
	int argc = 0;	//num of arguments
	int length = 0;	//records number of chars in storage array, s[]
	int reValue;	//return value from getword()

	while(true){
		reValue = getword(s + length);
		if(reValue == -255){	//return -255 if EOF
			newargv[argc] = NULL;
			return abs(reValue);
		}
		else if(reValue == 0){//finish collecting words at '$'
			newargv[argc] = NULL;
			return argc;
		}
		else if(reValue == 1){	//handle metacharacters |,>,<,&
			if(s[length] == '<'){
				if(flag_in >= 1){//more than one '<'
					return -2;	//redirection error
				}
				else{
					flag_in = 1;
					length = length+2;
					reValue = getword(s + length);	//check following word

					if(reValue == 0 || s[length] == '|' || s[length] == '<' || s[length] == '>' || reValue == -255){
						return -2;	//-2 indicating redirection error
					}

					input_file = (s+length);	//pointing input ptr to the string on s
					length = length + abs(reValue) + 1;
					continue;
				}
			}
			else if(s[length]=='>'){
				if(flag_out >= 1){	//redirect error when more than one '>'
					return -2;
				}
				else{
					flag_out = 1;
					length = length+2;
					reValue = getword(s + length);

					if(reValue == 0 || s[length] == '|' || s[length] == '<' || s[length] == '>' || reValue == -255){
						return -2;	//-2 indicating redirection error
					}

					output_file = (s+length);
					length = length+abs(reValue)+1;
					continue;
				}


			}
			else if(s[length]=='|'){
				if(flag_pipeline >= 1){//if theres already one pipe
					return -3;
				}

				flag_pipeline = 1;
				pipeIndex = argc;
				newargv[argc++] = NULL;
				length = length+2;
				continue;
			}
			else if(s[length]=='&' && amp_flag == 1){//& before newline
				newargv[argc] = NULL;
				return argc;
			}
			else{	//otherwise treat this char as an arg
				newargv[argc++] = (s+length);
				length = length + abs(reValue) + 1;
				continue;
			}
		}//end handling of |,<,>,&
		else{	//encountering normal words
			newargv[argc++] = (s+length);
			length = length + abs(reValue) + 1;
		}
	}
}

void myHandler(int signum){//catch signal, do nothing

}




