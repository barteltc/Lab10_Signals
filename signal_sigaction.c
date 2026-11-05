/**
 * File: signal_sigaction.c
 * Modified by: Chase Bartelt
 *
 * Brief summary of program: Registers a signal handler that displays PID
 * Compile: gcc -o signal_sigaction signal_sigaction.c
 *
 * Send signal: kill -USR1 <PID>
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void handle_signal(int sig, siginfo_t *info, void *context)
{
    (void)context;

    printf("Received signal %d from PID: %ld\n",
           sig, (long)info->si_pid);
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_sigaction = handle_signal;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    // Register the signal handler for SIGUSR1
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    // Wait for a signal to be received
    printf("Waiting for SIGUSR1... My PID: %ld\n", (long)getpid());

    while (1)
    {
        pause();
    }

    return 0;
}