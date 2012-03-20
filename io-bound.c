/*
 * File: mixed.c
 * Author: Alex Beal
 * Based on a Pi approximator by Andy Sayler
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2012/03/07
 * Modify Date: 2012/03/16
 * Description:
 *  This file contains an io bound bencharmark. It forks off the specified number of
 *  children, where each child copies the source file once.
 */

//#define DEBUG

/* Local Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_FILENAME_LEN 20
#define MIN_ARGS 5
#define USAGE "Usage: io-bound SCHED_POLICY SRC DEST NUM\n"

double calcPi(long, char*);
void childTask(char*, char*);
int consFileName(char*, char*, int, int);

int main(int argc, char* argv[]){

    int i;
    int pid;
    struct sched_param param;
    int policy;
    int child_count;
    int file_counter = 0;
    FILE* src;
    char dest_name[MAX_FILENAME_LEN];

    /* Process program arguments to select iterations and policy */
    if(argc < MIN_ARGS){
        fprintf(stderr, "Not enough arguments: %d\n", argc-1);
        fprintf(stderr, USAGE);
        exit(EXIT_FAILURE);
    }

    if(!strcmp(argv[1], "SCHED_OTHER")){
        policy = SCHED_OTHER;
    } else if(!strcmp(argv[1], "SCHED_FIFO")){
        policy = SCHED_FIFO;
    } else if(!strcmp(argv[1], "SCHED_RR")){
        policy = SCHED_RR;
    } else{
        fprintf(stderr, "Unhandeled scheduling policy\n");
        exit(EXIT_FAILURE);
    }

    /* Test that you can open the src file */
    src = fopen(argv[2], "r");
    if(src == NULL) {
        fprintf(stderr, "There was an error opening the source file: %s\n", argv[2]);
        perror("");
        exit(EXIT_FAILURE);
    }
    fclose(src);

    /* Set process to max prioty for given scheduler */
    param.sched_priority = sched_get_priority_max(policy);

    /* Set new scheduler policy */
    fprintf(stdout, "Current Scheduling Policy: %d\n", sched_getscheduler(0));
    fprintf(stdout, "Setting Scheduling Policy to: %d\n", policy);
    if(sched_setscheduler(0, policy, &param)){
        perror("Error setting scheduler policy");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "New Scheduling Policy: %d\n", sched_getscheduler(0));

    child_count = atoi(argv[4]);
    if( child_count < 1 || child_count > 5000 )
    {
        fprintf(stderr, "Oh really. %d children? I'm going to save you from yourself.", child_count);
        exit(EXIT_FAILURE);
    }

    int* children = malloc(sizeof(int)*child_count);

    /* Spawn children */
    fprintf(stdout, "Forking off children.\n");
    fflush(0); //Flush before forking!
    for(i=0; i<child_count; i++){
        pid = fork();
        if(pid > 0){
            /* Each child gets a different value file_counter which
             * will be used as a suffix for the dest files */
            file_counter += 1;
            children[i] = pid;
        } else if(pid == 0) {
            consFileName(dest_name, argv[3], file_counter, MAX_FILENAME_LEN);
            childTask(argv[2], dest_name);
            _exit(0);
        } else if(pid < 0) {
            fprintf(stderr, "Error forking. Bye.\n");
            exit(EXIT_FAILURE);
        }
    }

    for(i=0; i<child_count; i++){
        waitpid(children[i], NULL, 0);
    }
    free(children);

    printf("\n");

    return 0;
}

void childTask(char* src_name, char* dest_name) {
    FILE * src;
    FILE * dest;
    char buf;
    /* Open source file. */
    src = fopen(src_name, "r");
    if( src == NULL) {
        fprintf(stderr, "There was an error opening the source file.\n");
        perror("");
        exit(EXIT_FAILURE);
    }
    /* Open/create dest file. */
    dest = fopen(dest_name, "w");
    if( dest == NULL) {
        fprintf(stderr, "There was an error opening the destination file.\n");
        perror("");
        exit(EXIT_FAILURE);
    }
    /* copy copy copy. */
    while( (buf = fgetc(src)) != EOF ){
        fputc(buf, dest);
        /* Copy one character at a time. Ouch! */
        fflush(dest);
    }
    /* Close dest and src. */
    fclose(dest);
    fclose(src);
}

int consFileName(char* dest, char* prefix, int suffix, int max_len){
    char name[MAX_FILENAME_LEN] = "";
    char unique_str[MAX_FILENAME_LEN - 5];

    sprintf(unique_str, "%d", suffix);
    strncat(name, prefix, MAX_FILENAME_LEN);
    strncat(name, unique_str, MAX_FILENAME_LEN);

    strncpy(dest, name, max_len);

    return 0;
}
