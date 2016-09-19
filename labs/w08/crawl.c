// crawl.c ... build a graph of part of the web
// Written by John Shepherd, September 2015
// Uses the cURL library and functions by Vincent Sanders <vince@kyllikki.org>
// Code by Jamie Priest and Nithin Sudhir
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <curl/curl.h>
#include <string.h>
#include "stack.h"
#include "queue.h"
#include "set.h"
#include "graph.h"
#include "html.h"
#include "url_file.h"

#define BUFSIZE 1024

void setFirstURL(char *, char *);
void normalise(char *, char *, char *, char *, int);

int main(int argc, char **argv)
{
	URL_FILE *handle;
	char buffer[BUFSIZE]; //buffer that store line of html text
	char baseURL[BUFSIZE];// the parent url
	char firstURL[BUFSIZE];// store the firstURL need to store in the set
	char next[BUFSIZE]; // used to iterate through the web
	int  maxURLs; // number of urls are allowed in this ex

	//make baseURL as input, and assign maxURLS
	if (argc > 2) {
		char approval;
		if (strstr(argv[1],"unsw.edu.au")==NULL) {
			printf("WARNING: Running webcrawlers may be illegal on this domain, do you wish to continue\n Please type 'y' or 'n'\n");
			while( approval != 'y' || approval != 'n'){
				scanf("%c", &approval);
				if (approval == 'y') {
			// warned you
				break;
				}
				else if( approval == 'n'){
					return EXIT_SUCCESS;
				}
			}
		}

		strcpy(baseURL,argv[1]);
		setFirstURL(baseURL,firstURL); //normalise url so it can be used
		printf("%s\n",firstURL);
		maxURLs = atoi(argv[2]);
		if (maxURLs < 40) maxURLs = 40;
	}
	else {
		fprintf(stderr, "Usage: %s BaseURL MaxURLs\n",argv[0]);
		exit(1);
	}

<<<<<<< Updated upstream
=======
//<<<<<<< HEAD
>>>>>>> Stashed changes
	Set seen = newSet();//initialise set of Seen URLs
	Graph web = newGraph(maxURLs);//initialise Graph to hold up to maxURLs
	Queue wait = newQueue(); //add firstURL to the ToDo list
	enterQueue(wait,firstURL);


	while(!emptyQueue(wait) && nVertices(web) != maxURLs){
		strcpy(next, leaveQueue(wait));
		if (!(handle = url_fopen(next, "r"))) {
			fprintf(stderr,"Couldn't open %s\n", next);
			continue;
		}
<<<<<<< Updated upstream
		//foreach line in the opened URL {
=======
//=======
//>>>>>>> origin/master

		Set seen = newSet();//initialise set of Seen URLs
		Graph web = newGraph(maxURLs);//initialise Graph to hold up to maxURLs
		Queue wait = newQueue(); //add firstURL to the ToDo list
		enterQueue(wait,firstURL);

//<<<<<<< HEAD
				//if (this URL not Seen already) {
	            //add it to the Seen set
	            //add it to the ToDo list
	            if (!(isElem(seen,result))){
	            	insertInto(seen,result);//make the url found into seen list
								enterQueue(wait,result);
	            }
				//if (Graph not filled or both URLs in Graph)
	            //add an edge from Next to this URL
				if (!isConnected(web,next,result)) addEdge(web,next,result);
//=======
		while(!emptyQueue(wait) && nVertices(web) != maxURLs){
			strcpy(next, leaveQueue(wait));
			// strcpy(next, popFrom(wait));
			if (!(handle = url_fopen(next, "r"))) {
				fprintf(stderr,"Couldn't open %s\n", next);
				continue;
			}
			//foreach line in the opened URL {
>>>>>>> Stashed changes
			while(!url_feof(handle)) {
				url_fgets(buffer,sizeof(buffer),handle); //get line of input
				//fputs(buffer,stdout);
				int pos = 0; //position of the web pointer
				char result[BUFSIZE];// used to store url found in the current html
<<<<<<< Updated upstream
=======
//>>>>>>> origin/master
>>>>>>> Stashed changes
				memset(result,0,BUFSIZE);
				//foreach URL on that line
				while ((pos = GetNextURL(buffer, next, result, pos)) > 0) {
					if( strstr( result, "unsw.edu.au" ) == NULL  ) continue;
					//printf("Found: '%s'\n",result);

					//if (this URL not Seen already) {
		        		    //add it to the Seen set
			            	//add it to the ToDo list
				            if (!(isElem(seen,result))){
				       	     	insertInto(seen,result);//make the url found into seen list
						enterQueue(wait,result);
				            }
					//if (Graph not filled or both URLs in Graph)
				            //add an edge from Next to this URL
					if (!isConnected(web,next,result)) addEdge(web,next,result);
					memset(result,0,BUFSIZE);
				}
			}
			url_fclose(handle);
			sleep(1);
		}

		showGraph(web,1);
		showGraph(web,2);
		return 0;
	}

	void setFirstURL(char *base, char *first) {
		char *c;
		if ((c = strstr(base, "/index.html")) != NULL) {
			strcpy(first,base);
			*c = '\0';
		}
		else if (base[strlen(base)-1] == '/') {
			strcpy(first,base);
			strcat(first,"index.html");
			base[strlen(base)-1] = '\0';
		}
		else {
			strcpy(first,base);
			strcat(first,"/index.html");
		}
	}
