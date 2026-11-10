/**
 * File: tennis_recieve.c
 * Modified by: Chase Bartelt
 *
 * Returns the ball and keeps the same score as the sender.
 * Compile: gcc -Wall -Wextra -o r tennis_recieve.c
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t incoming;
static volatile sig_atomic_t sender;

// Save the received value and the process that sent it.
static void handle_signal(int sig, siginfo_t *info, void *context)
{
    (void)sig;
    (void)context;

    if (info->si_code == SI_QUEUE)
    {
        incoming = info->si_value.sival_int;
        sender = info->si_pid;
    }
}

// Every line uses the same three columns.
static void print_row(const char *event, const char *player1,
                      const char *player2)
{
    printf("%-15s | Player 1 %-9s | Player 2 %-9s\n",
           event, player1, player2);
}

static void print_shot(int shot, int player, int hitOrMiss)
{
    char label[32];
    snprintf(label, sizeof(label), "Shot %d", shot);

    const char *result = "hit";
    if (hitOrMiss == 0)
    {
        result = "missed";
    }

    if (player == 1)
    {
        print_row(label, result, "-");
    }
    else
    {
        print_row(label, "-", result);
    }
}

static int print_score(int score1, int score2)
{
    const char *score[] = {"Love", "15", "30", "40"};

    if (score1 >= 4 && score1 - score2 >= 2)
    {
        print_row("Game over", "wins", "-");
        return 1;
    }

    if (score2 >= 4 && score2 - score1 >= 2)
    {
        print_row("Game over", "-", "wins");
        return 1;
    }

    if (score1 >= 3 && score2 >= 3)
    {
        if (score1 == score2)
        {
            print_row("Deuce", "40", "40");
        }
        else if (score1 > score2)
        {
            print_row("Score", "Advantage", "40");
        }
        else
        {
            print_row("Score", "40", "Advantage");
        }
    }
    else
    {
        print_row("Score", score[score1], score[score2]);
    }

    return 0;
}

int main(void)
{
    int score1 = 0;
    int score2 = 0;
    int shot = 0;
    int turn = 0;

    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    // Hold incoming signals until we are ready to receive them.
    sigset_t ballMask, waitMask;
    sigemptyset(&ballMask);
    sigaddset(&ballMask, SIGRTMIN);

    if (sigprocmask(SIG_BLOCK, &ballMask, &waitMask) == -1)
    {
        perror("sigprocmask");
        return 1;
    }
    sigdelset(&waitMask, SIGRTMIN);

    struct sigaction sa = {0};
    sa.sa_sigaction = handle_signal;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGRTMIN, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    printf("Receiver PID: %ld\n", (long)getpid());
    fflush(stdout);

    while (1)
    {
        while (sender == 0)
        {
            sigsuspend(&waitMask);
        }

        pid_t serverPid = (pid_t)sender;
        sender = 0;

        // The sender uses 2 to request the ten-exchange demo.
        int volleyOnly = 0;

        if (incoming == 2)
        {
            volleyOnly = 1;
            incoming = 1;
        }

        shot++;
        print_shot(shot, 1, incoming);

        // Player 1 missed. Award the point and wait for another serve.
        if (incoming == 0)
        {
            score2++;

            if (print_score(score1, score2))
            {
                return 0;
            }

            continue;
        }

        // Attempt to return the ball.
        sleep(1 + rand() % 2);
        int hitOrMiss = 1;

        if (volleyOnly == 0)
        {
            hitOrMiss = rand() % 2;
        }

        union sigval reply;
        reply.sival_int = hitOrMiss;

        if (sigqueue(serverPid, SIGRTMIN, reply) == -1)
        {
            perror("sigqueue");
            return 1;
        }

        shot++;
        print_shot(shot, 2, hitOrMiss);

        if (hitOrMiss == 0)
        {
            score1++;

            if (print_score(score1, score2))
            {
                return 0;
            }
        }
        else
        {
            putchar('\a');
            fflush(stdout);
        }

        // Count a completed exchange in demo mode.
        if (volleyOnly == 1)
        {
            turn++;
            printf("Exchange %d/10 complete.\n", turn);

            if (turn == 10)
            {
                return 0;
            }
        }
    }
}