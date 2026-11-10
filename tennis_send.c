/**
 * File: tennis_send.c
 * Modified by: Chase Bartelt
 *
 * Plays one tennis game or a 10-exchange demonstration.
 * Compile: gcc -Wall -Wextra -o s tennis_send.c
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3 ||
        (argc == 3 && strcmp(argv[2], "--volley") != 0))
    {
        printf("Usage: %s PID [--volley]\n", argv[0]);
        return 1;
    }

    pid_t pid = (pid_t)atoi(argv[1]);

    if (pid <= 0 || pid == getpid())
    {
        puts("Enter the PID printed by the receiver.");
        return 1;
    }

    int score1 = 0;
    int score2 = 0;
    int shot = 0;
    int turn = 0;
    int serving = 1;
    int volleyOnly = (argc == 3);

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

    while (1)
    {
        sleep(1 + rand() % 2);
        int hitOrMiss = 1;

        if (volleyOnly == 0)
        {
            hitOrMiss = rand() % 2;

            // Tennis allows a second serve after the first one misses.
            if (serving == 1 && hitOrMiss == 0)
            {
                sleep(1 + rand() % 2);
                hitOrMiss = rand() % 2;
            }
        }

        union sigval value;
        value.sival_int = hitOrMiss;

        if (volleyOnly == 1)
        {
            value.sival_int = 2; // Guaranteed ball for the demo.
        }

        if (sigqueue(pid, SIGRTMIN, value) == -1)
        {
            perror("sigqueue");
            return 1;
        }

        shot++;
        print_shot(shot, 1, hitOrMiss);

        // During a serve, this means both attempts missed.
        if (hitOrMiss == 0)
        {
            score2++;

            if (print_score(score1, score2))
            {
                return 0;
            }

            serving = 1;
            continue;
        }

        putchar('\a');
        fflush(stdout);
        serving = 0;

        // Wait for Player 2's return or miss.
        while (sender != pid)
        {
            sigsuspend(&waitMask);
        }
        sender = 0;

        shot++;
        print_shot(shot, 2, incoming);

        if (incoming == 0)
        {
            score1++;

            if (print_score(score1, score2))
            {
                return 0;
            }

            serving = 1;
        }

        // Demo mode ends after ten complete trips there and back.
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