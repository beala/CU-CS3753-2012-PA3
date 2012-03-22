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
#define _GNU_SOURCE

/* Local Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_FILENAME_LEN 20
#define MIN_ARGS 7
#define USAGE "Usage: io-bound SCHED_POLICY SRC DEST BLOCK_SIZE TRAN_SIZE NUM\n"

void childTask(size_t, size_t, char*, char*);
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
    size_t block_size;
    ssize_t tran_size;

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

    if(atoi(argv[4]) < 0){
        fprintf(stderr, "Invalid block size.\n");
        exit(EXIT_FAILURE);
    } else {
        block_size = atoi(argv[4]);
    }

    if(atoi(argv[5]) < 0 || atoi(argv[5]) < (int)block_size){
        fprintf(stderr, "Invalid transfer size.\n");
        exit(EXIT_FAILURE);
    } else {
        tran_size = atoi(argv[5]);
    }

    child_count = atoi(argv[6]);
    if( child_count < 1 || child_count > 5000 )
    {
        fprintf(stderr, "Too many or too few children procs: %d\n", child_count);
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
            childTask(block_size, tran_size, argv[2], dest_name);
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

void childTask(size_t b_size, size_t t_size, char* src_name, char* dest_name) {
    int srcFD;
    int destFD;
    ssize_t bytes_read = 0;
    ssize_t bytes_written = 0;
    ssize_t total_bytes_written = 0;
    char* buf = NULL;
    /* Open source file. */
    srcFD = open(src_name, O_RDONLY | O_SYNC);
    if( srcFD < 0) {
        fprintf(stderr, "There was an error opening the source file.\n");
        perror("");
        exit(EXIT_FAILURE);
    }
    /* Open/create dest file. */
    destFD = open(dest_name, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if( destFD < 0) {
        fprintf(stderr, "There was an error opening the destination file.\n");
        perror("");
        exit(EXIT_FAILURE);
    }

    /* malloc buffer */
    buf = malloc(b_size*sizeof(char));
    if(buf == NULL) {
        fprintf(stderr, "Error allocating buffer.\n");
        exit(EXIT_FAILURE);
    }
    do {
        bytes_read = read(srcFD, buf, b_size);
        if(bytes_read < 0){
            perror("Error reading source file.");
            exit(EXIT_FAILURE);
        }

        if(bytes_read > 0) {
            bytes_written = write(destFD, buf, bytes_read);
            if(bytes_written < 0){
                perror("Error writing to output file.");
                exit(EXIT_FAILURE);
            } else {
                total_bytes_written += bytes_written;
            }
        }
        if(bytes_read != (ssize_t)b_size){
            if(lseek(srcFD, 0, SEEK_SET)){
                perror("Error seeking to beginning of file.");
                exit(EXIT_FAILURE);
            }
        }
    } while(total_bytes_written < (ssize_t)t_size);

    if(close(srcFD)){
        perror("Error closing source file.");
        exit(EXIT_FAILURE);
    }
    if(close(destFD)){
        perror("Error closing dest file.");
        exit(EXIT_FAILURE);
    }

    free(buf);
}

int consFileName(char* dest, char* prefix, int suffix, int max_len){
    char name[MAX_FILENAME_LEN] = "";
    char unique_str[MAX_FILENAME_LEN - 5];

    sprintf(unique_str, "%d", suffix);
    if(strlen(unique_str) + strlen(prefix) > MAX_FILENAME_LEN){
        fprintf(stderr, "Output filename too long.\n");
        exit(EXIT_FAILURE);
    }
    strncat(name, prefix, MAX_FILENAME_LEN);
    strncat(name, unique_str, MAX_FILENAME_LEN);

    strncpy(dest, name, max_len);

    return 0;
}
