/*
 * File: io-bound.c
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

/* Local Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include "entropy_src.h"

#define DEFAULT_ITERATIONS 1000000
#define DEFAULT_CHILDREN 10
#define RADIUS (RAND_MAX / 2)
#define TMP_DIR "/tmp/"
#define DEFAULT_TMP_NAME "mixed"
#define MAX_FILENAME_LEN 20

inline double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

inline double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}

double calcPi(long, char*);
void childTask(long, int);

int main(int argc, char* argv[]){

    int i;
    int pid;
    long iterations;
    struct sched_param param;
    int policy;
    int child_count;
    int entropy_i = 0;

    /* Process program arguments to select iterations and policy */
    /* Set default iterations if not supplied */
    if(argc < 2){
        iterations = DEFAULT_ITERATIONS;
    }
    /* Set default policy if not supplied */
    if(argc < 3){
        policy = SCHED_OTHER;
    }
    /* Set iterations if supplied */
    if(argc > 1){
        iterations = atol(argv[1]);
        if(iterations < 1){
            fprintf(stderr, "Bad iterations value\n");
            exit(EXIT_FAILURE);
        }
    }
    /* Set policy if supplied */
    if(argc > 2){
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

    if(argc > 3) {
        child_count = atoi(argv[3]);
        if( child_count < 1 || child_count > 5000 )
        {
            fprintf(stderr, "Oh really. %d children? I'm going to save you from yourself.", child_count);
            exit(EXIT_FAILURE);
        }
    } else {
        child_count = DEFAULT_CHILDREN;
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
            childTask(iterations, entropy_i);
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

void childTask(long iterations, int entropy_i) {
            /* Use unique entropy_i to index into entropy matrix to get
             * unique seed. */
            srand(entropy_mat[entropy_i]);
            double pi = calcPi(iterations, "/tmp/test1");
#ifdef DEBUG
            fprintf(stdout, "%g ",pi); 
            fflush(stdout);
#endif
            (void) pi;
}

double calcPi(long iter, char* tmp_fname) {

    long i;
    double x, y;
    double inCircle = 0.0;
    double inSquare = 0.0;
    double pCircle = 0.0;
    double piCalc = 0.0;
    FILE*  tmp_file;

    /* Calculate pi using statistical methode across all iterations*/
    for(i=0; i<iter; i++){
        tmp_file = fopen(tmp_fname, "w");
        x = (random() % (RADIUS * 2)) - RADIUS;
        fprintf(tmp_file, "%lg\n", x);
        fflush(tmp_file);
        y = (random() % (RADIUS * 2)) - RADIUS;
        fprintf(tmp_file, "%lg", y);
        fflush(tmp_file);
        fclose(tmp_file);
        tmp_file = fopen(tmp_fname, "r");
        fscanf(tmp_file, "%lg\n%lg", &x, &y);
        fclose(tmp_file);
        if(zeroDist(x,y) < RADIUS){
            inCircle++;
        }
        inSquare++;
    }

    /* Finish calculation */
    pCircle = inCircle/inSquare;
    piCalc = pCircle * 4.0;

    return piCalc;
}

int consFileName(char* dest, char* tmp_path, int unique, int max_len){
    char name[MAX_FILENAME_LEN] = "";
    char unique_str[MAX_FILENAME_LEN - 5];

    sprintf(unique_str, "%d", unique);
    strcat(name, tmp_path);
    strcat(name, DEFAULT_TMP_NAME);
    strcat(name, unique_str);

    strncpy(dest, name, max_len);

    return 0;
}
