/* 
Thomas Martinson
Carroll
CS570
Program 2
Due Date: 12/5/18
 */
 
 /*
 This file is used to count the number of characters in a word and
 stores each character in an array so that the program, p2.c, can print it out.
 */


#include "getword.h"


int getword( char *w )
{
	int iochar;                                           
	int wordlength = 0;
	char *path;
    
	/*Main loop which terminates when it reaches the end of file.*/
	while ( ( iochar = getchar() ) != EOF )
	{      
		//Makes sure that the arrays storage doesn't get full
		if(wordlength == 254){
		ungetc(iochar,stdin);
		return wordlength;
		}
	         
		/*Check in order to skip leading spaces.*/
		if ( iochar != ' ' )
		{
			if ( iochar == ';' || iochar == '\n' || iochar == '|' || iochar=='<' || iochar=='>' || iochar == '\\')
			{
				/*Puts either ; or \n back onto the input stream for next getchar() call.*/
				if( wordlength > 0 )
				{
					ungetc(iochar,stdin);
				}
				/* Handles \, not functioning
				if(iochar == '\\'){
					iochar = getchar();
					iochar=
					*w++ = iochar;
					
					*w = '\0';
					return wordlength;
				}
				*/

				/* Handles < and <<, not functioning
				if(iochar == '<')
				{
					int nextChar = getchar();
					if(nextChar=='<'){
						w[wordlength] = '\0';
						w[wordlength++] = nextChar;
					}else
					{
						
						ungetc(iochar,stdin);
					}
				}
				*/
				
				//Non-special meta-characters indicate a newline 
				if(iochar == '|' || iochar == '>' || iochar == '<')
				{
					if(wordlength<1){
						w[wordlength++] = iochar;
					}
					
				}
					
				w[wordlength] = '\0';
				
				/*If word starts with $, changes output length to negative.*/
				if ( wordlength > 0 && w[0] == '$' )
				{
					memmove ( w, w+1, wordlength );
					return ( ( wordlength - 1 ) * -1 );
				}
				
				//Returns string representing relative path of w
				if ( wordlength > 0 && w[0] == '~' )
				{
					memmove ( w, w+1, wordlength );
					path = getenv("HOME"); //Check HOME evironment variable for path
					strcat(path,w); //Adds w to end of path
					strcpy(w,path);
					return strlen(w);
				}
			 
				return wordlength;
			}
			w[wordlength++] = iochar;
			
		}
		
		/*Adds null terminator to signal start of new word.*/
		if( wordlength > 0 && iochar == ' '  )
		{
			
			w[wordlength] = '\0';
			
			/*If word starts with $, changes output length to negative.*/
			if ( w[0] == '$' )
			{
				memmove ( w, w+1, wordlength );
				return ( ( wordlength - 1 ) * -1 );
			}
			//Returns string representing relative path of w
				if ( wordlength > 0 && w[0] == '~' )
				{
					memmove ( w, w+1, wordlength );
					path = getenv("HOME"); //Check HOME evironment variable for path
					strcat(path,w); //Adds w to end of path
					strcpy(w,path);
					return strlen(w);
				}
			return wordlength;
		}
	}
	if(wordlength > 0)
	{
		/*If word starts with $, changes output length to negative.*/
		w[wordlength] = '\0';
		if(w[0] == '$')
		{
			memmove ( w, w+1, wordlength );
			return ( -1* (wordlength - 1) );
		}
		
		//Returns string representing relative path of w
		if ( wordlength > 0 && w[0] == '~' )
		{
			memmove ( w, w+1, wordlength );
			path = getenv("HOME"); //Check HOME evironment variable for path
			strcat(path,w); //Adds w to end of path
			strcpy(w,path);
			return strlen(w);
		}
		return wordlength;
	}
	
	/*Signals end of input, in order for program termination.*/
	if ( wordlength == 0 )
	{
		w[0] = '\0';
		return -255;
	}
}
	
	
 


