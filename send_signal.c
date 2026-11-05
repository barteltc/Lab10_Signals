/**
 * File: send_signal.c
 * Modified by: Chase Bartelt
 *
 * Brief summary: Sends a random integer
 * to another process using SIGUSR1.
 *
 * Compile: gcc -o send_signal send_signal.c
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <PID>\n", argv[0]);
        return 1;
    }

    // Convert argument to process ID
    pid_t pid = (pid_t)atoi(argv[1]);

    // Seed random number generator
    srand(time(NULL));

    // Generate random number
    int number = rand();

    union sigval value;
    value.sival_int = number;

    printf("Sending integer: %d\n", number);

    // Send the signal with the random number
    if (sigqueue(pid, SIGUSR1, value) == -1)
    {
        perror("sigqueue");
        return 1;
    }
    printf("Sending integer: %d\n", number);

    return 0;
}