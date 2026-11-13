/**
 * File: recv_signal.c
 * Modified by: Chase Bartelt
 *
 * Brief summary: Receives SIGUSR1 and prints
 * the integer sent using sigqueue.
 *
 * Compile: gcc -o recv_signal recv_signal.c
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void handle_signal(int sig, siginfo_t *info, void *context)
{
    (void)sig;
    (void)context;

    printf("Received integer: %d\n",
           info->si_value.sival_int);
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_sigaction = handle_signal;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    // Register SIGUSR1 handler
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    printf("Receiver PID: %ld\n", (long)getpid());
    fflush(stdout);

    while (1)
    {
        pause();
        exit(0);
    }
}