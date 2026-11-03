/**
 * @file signal_segfault.c
 * @brief Program that dereferences a null pointer causing a segmentation fault
 */

/**
 * Modified by: Chase Bartelt
 *
 * Brief summary of modifications: Receives a SIGSEGV signal
 */

#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void handle_signal(int sig)
{
    (void)sig;
    printf("Received SIGSEGV\n");
}

int main(void)
{
    // Register the signal handler for SIGSEGV
    signal(SIGSEGV, handle_signal);

    // Declare a null pointer
    int *i = NULL;

    // Dereference the null pointer
    printf("The value of i is: %d\n", *i);

    // Return to exit the program
    return 0;
}