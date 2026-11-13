/**
 * File: tennis_recieve.c
 * Modified by: Chase Bartelt
 *
 * Returns the ball and keeps the same score as the sender.
 * Compile: gcc -Wall -Wextra -o r tennis_recieve.c
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define _POSIX_C_SOURCE 202405L
// Player 2's chance of hitting each return.
#define HIT_PERCENT 80

// The signal handler writes these values. The main loop reads them.
static volatile sig_atomic_t incoming;
static volatile sig_atomic_t sender;

static void handle_signal(int sig, siginfo_t *info, void *context)
{
    (void)sig;
    (void)context;

    // A signal sent with sigqueue carries the shot result and the sender's PID.
    if (info->si_code == SI_QUEUE)
    {
        incoming = info->si_value.sival_int;
        sender = info->si_pid;
    }
}

// Keep the event and both player columns aligned.
static void print_row(const char *event, const char *player1,
                      const char *player2)
{
    printf("%-15s | Player 1 %-9s | Player 2 %-9s\n",
           event, player1, player2);
}

// Show the result in the column of the player who took the shot.
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

// Convert point totals to tennis scores. Return 1 when someone wins.
static int print_score(int score1, int score2)
{
    const char *score[] = {"Love", "15", "30", "40"};

    // A player needs at least four points and a two-point lead to win.
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

    // Once both players reach 40, a tie is deuce and a lead is advantage.
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

    // Mix the time and PID so both processes can start with different seeds.
    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    // Block the ball signal during normal work. sigsuspend unblocks it
    // while waiting so a shot that arrives early stays pending.
    sigset_t ballMask, waitMask;
    sigemptyset(&ballMask);
    sigaddset(&ballMask, SIGRTMIN);

    if (sigprocmask(SIG_BLOCK, &ballMask, &waitMask) == -1)
    {
        perror("sigprocmask");
        return 1;
    }
    sigdelset(&waitMask, SIGRTMIN);

    // SA_SIGINFO lets the handler read the queued value and sender's PID.
    struct sigaction sa = {0};
    sa.sa_sigaction = handle_signal;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGRTMIN, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    // Player 1 uses this PID to send the first shot.
    printf("Receiver PID: %ld\n", (long)getpid());
    fflush(stdout);

    while (1)
    {
        while (sender == 0)
        {
            sigsuspend(&waitMask);
        }

        // Save the PID for the reply, then clear it for the next shot.
        pid_t serverPid = (pid_t)sender;
        sender = 0;

        // Signal values are 0 for a miss, 1 for a hit, and 2 for a demo hit.
        int volleyOnly = 0;

        if (incoming == 2)
        {
            volleyOnly = 1;
            incoming = 1;
        }

        shot++;
        print_shot(shot, 1, incoming);

        // A Player 1 miss gives Player 2 the point. Wait for the next serve.
        if (incoming == 0)
        {
            score2++;

            if (print_score(score1, score2))
            {
                return 0;
            }

            continue;
        }

        sleep(1 + rand() % 2);
        int hitOrMiss = 1;

        if (volleyOnly == 0)
        {
            // A roll from 0 to 99 succeeds when it is below HIT_PERCENT.
            hitOrMiss = (rand() % 100 < HIT_PERCENT);
        }

        // Send the result back so Player 1 can update its copy of the score.
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
            // Ring the terminal bell for a successful return.
            putchar('\a');
            fflush(stdout);
        }

        // Stop after the tenth return in demo mode.
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