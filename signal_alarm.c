/**
 * @file signal_alarm.c
 * @brief Sets a signal handler for SIGALRM, the handler prints a message and then quits
 */

/**
 * Modified by: Chase Bartelt
 *
 * Brief summary of modifications: Added an alarm
 */

#include <signal.h>
#include <unistd.h>

/**
 * @brief Signal handler for SIGALRM - prints a message and exits
 */
void handle_signal(int sig)
{
    (void)sig;
    write(STDOUT_FILENO, "Received SIGALRM\n", 17);
}

int main(void)
{
    // Register the SIGALRM handler
    signal(SIGALRM, handle_signal);

    // Schedule an alarm for 5 seconds
    alarm(5);

    // Wait until the alarm arrives
    pause();

    return 0;
}