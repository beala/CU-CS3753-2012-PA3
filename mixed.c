/*
 * File: mixed.c
 * Author: Alex Beal
 * Based on a Pi approximator by Andy Sayler
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2012/03/07
 * Modify Date: 2012/03/16
 * Description:
 *  This file contains an io bound bencharmark. It forks off the specified number of
 *  children, where each child generates an approximation of Pi.
 */

//#define DEBUG
#define _GNU_SOURCE

/* Local Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include "entropy_src.h"

#define DEFAULT_ITERATIONS 1000000
#define DEFAULT_CHILDREN 10
#define RADIUS (RAND_MAX / 2)
#define TMP_DIR "/tmp/"
#define DEFAULT_TMP_NAME "mixed"
#define MAX_FILENAME_LEN 20
#define MIN_ARGS 8
#define USAGE "./mixed ITER SCHED_POLICY PROCS BLOCK TOTAL SRC DEST"

inline double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

inline double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}

double calcPi(long);
void childTask(long, int, char*, char*, size_t, size_t);
int consFileName(char*, char*, int, int);
void runDisk(size_t, size_t, char*, char*);

int main(int argc, char* argv[]){

    int i;
    int pid;
    long iterations;
    struct sched_param param;
    int policy;
    int child_count;
    int entropy_i = 0;

    size_t tran_size = 0;
    size_t block_size = 0;
    char dest[MAX_FILENAME_LEN]= "";

    if(argc < MIN_ARGS){
        fprintf(stderr,"Too few arguments.\n%s", USAGE);
        exit(EXIT_FAILURE);
    }

    /* Set iterations if supplied */
    iterations = atol(argv[1]);
    if(iterations < 100){
        fprintf(stderr, "Bad iterations value. Must be at least 100.\n");
        exit(EXIT_FAILURE);
    }
    /* Set policy if supplied */
    if(!strcmp(argv[2], "SCHED_OTHER")){
        policy = SCHED_OTHER;
    }
    else if(!strcmp(argv[2], "SCHED_FIFO")){
        policy = SCHED_FIFO;
    }
    else if(!strcmp(argv[2], "SCHED_RR")){
        policy = SCHED_RR;
    }
    else{
        fprintf(stderr, "Unhandeled scheduling policy\n");
        exit(EXIT_FAILURE);
    }

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


    child_count = atoi(argv[3]);
    if( child_count < 1 || child_count > 5000 )
    {
        fprintf(stderr, "Invalid process count: %d.\n", child_count);
        exit(EXIT_FAILURE);
    }

    if(atoi(argv[4]) < 0){
        fprintf(stderr, "Invalid block size.\n");
        exit(EXIT_FAILURE);
    } else {
        block_size = atoi(argv[4]);
    }

    if(atoi(argv[5]) < 0 || atoi(argv[5])/100 < (int)block_size){
        fprintf(stderr, "Invalid transfer size.\n");
        exit(EXIT_FAILURE);
    } else {
        tran_size = atoi(argv[5]);
    }

    int* children = malloc(sizeof(int)*child_count);

    /* Spawn children */
    fprintf(stdout, "Forking off children.\n");
    fflush(0); //Flush before forking!
    for(i=0; i<child_count; i++){
        pid = fork();
        if(pid > 0){
            /* Each child gets a different value for entropy_i.*/
            entropy_i += 1;
            entropy_i %= ENTROPY_MAT_LEN;
            children[i] = pid;
        } else if(pid == 0) {
            consFileName(dest, argv[7], entropy_i, MAX_FILENAME_LEN);
            childTask(iterations, entropy_i, argv[6], dest, block_size, tran_size);
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

void childTask(long iterations, int entropy_i, char* src, char* dest, size_t b_size, size_t t_size){
    int i = 0;
    srand(entropy_mat[entropy_i]);
    for(i=0; i < 100; i++){
        calcPi(iterations/100);
        runDisk(b_size, t_size/100, src, dest);
    }
}

double calcPi(long iter) {

    long i;
    double x, y;
    double inCircle = 0.0;
    double inSquare = 0.0;
    double pCircle = 0.0;
    double piCalc = 0.0;

    /* Calculate pi using statistical methode across all iterations*/
    for(i=0; i<iter; i++){
        x = (random() % (RADIUS * 2)) - RADIUS;
        y = (random() % (RADIUS * 2)) - RADIUS;
        if(zeroDist(x,y) < RADIUS){
            inCircle++;
        }
        inSquare++;
    }

    pCircle = inCircle/inSquare;
    piCalc = pCircle * 4.0;

    return piCalc;
}

void runDisk(size_t b_size, size_t t_size, char* src_name, char* dest_name) {
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
int consFileName(char* dest, char* tmp_path, int unique, int max_len){
    char name[MAX_FILENAME_LEN] = "";
    char unique_str[MAX_FILENAME_LEN - 5];

    sprintf(unique_str, "%d", unique);
    strcat(name, tmp_path);
    strcat(name, unique_str);

    strncpy(dest, name, max_len);

    return 0;
}
