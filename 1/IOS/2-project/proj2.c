#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Gleb Shmonin (xshmon00)

// Declaration of functions
int init_vars(int Z);
void destroy_vars();
int lyzars(int idL);
int skibus();

FILE *fp;

// Shared variables
int L, Z, K, TL, TB;
int *output_cnt = 0;
int *zastavka_capacity = 0;
int *transported_skiers = 0;

// Semaphores
sem_t *output;                  // Semaphore for output
sem_t *bus_arrived_to_zastavka; // Semaphore for bus arrival to stop that stops skier process from coming to stop
sem_t *boarding;                // Semaphore to signal that skier has boarded to bus
sem_t *terminus;                // Semaphore for final stop that signals skiers to go to ski
sem_t *going_ski;               // Semaphore aslo for final stop to signal bus that skier has went to ski
sem_t *zastavka_sem[10];        // Semaphores for all stops that signals for each stop that bus has arrived

// Initialization of variables
int init_vars(int Z)
{

    // Semaphores
    bus_arrived_to_zastavka = mmap(NULL, sizeof(bus_arrived_to_zastavka), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(bus_arrived_to_zastavka, 1, 1);
    output = mmap(NULL, sizeof(output), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(output, 1, 1);
    terminus = mmap(NULL, sizeof(terminus), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(terminus, 1, 0);
    boarding = mmap(NULL, sizeof(boarding), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(boarding, 1, 0);
    going_ski = mmap(NULL, sizeof(boarding), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(going_ski, 1, 0);

    // Semaphores for stops
    for (int i = 0; i < Z; i++)
    {
        zastavka_sem[i] = mmap(NULL, sizeof(zastavka_sem[i]), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        sem_init(zastavka_sem[i], 1, 0);

        if (zastavka_sem[i] == NULL)
        {
            destroy_vars();
            fprintf(stderr, "Initialization variables Error\n");
            exit(EXIT_FAILURE);
        }
    }

    // Shared variables
    output_cnt = mmap(NULL, sizeof(output_cnt), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    transported_skiers = mmap(NULL, sizeof(output_cnt), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    zastavka_capacity = mmap(NULL, 10, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    setbuf(fp, NULL);

    if (bus_arrived_to_zastavka == NULL || output == NULL || terminus == NULL || boarding == NULL || going_ski == NULL ||
        output_cnt == NULL || transported_skiers == NULL || zastavka_capacity == NULL)
    {
        destroy_vars();
        fprintf(stderr, "Initialization variables Error\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

// Destroying
void destroy_vars()
{
    // Semaphores
    sem_destroy(bus_arrived_to_zastavka);
    munmap(bus_arrived_to_zastavka, sizeof(bus_arrived_to_zastavka));
    sem_destroy(output);
    munmap(output, sizeof(output));
    sem_destroy(terminus);
    munmap(terminus, sizeof(terminus));
    sem_destroy(boarding);
    munmap(boarding, sizeof(boarding));
    sem_destroy(going_ski);
    munmap(going_ski, sizeof(going_ski));
    for (int i = 0; i < Z; i++)
    {
        sem_destroy(zastavka_sem[i]);
        munmap(zastavka_sem[i], sizeof(zastavka_sem[i]));
    }

    // Shared variables
    munmap(output_cnt, sizeof(output_cnt));
    munmap(transported_skiers, sizeof(transported_skiers));
    munmap(zastavka_capacity, sizeof(zastavka_capacity));
    return;
}

// Child process for skier
int lyzars(int idL)
{
    sem_wait(output);
    fprintf(fp, "%i: L %i: started\n", ++(*output_cnt), idL);
    sem_post(output);

    // Get random time to eat breakfast
    srand(time(NULL) * getpid());
    usleep((rand() % TL + 1));

    // Get random number for skier's stop
    int waits_on_zastavka = (rand() % Z) + 1;

    // Semaphore for critical section when bus arrives
    sem_wait(bus_arrived_to_zastavka);

    sem_wait(output);
    fprintf(fp, "%i: L %i: arrived to %i\n", ++(*output_cnt), idL, waits_on_zastavka);
    sem_post(output);

    // Increment number of skiers on stop
    zastavka_capacity[waits_on_zastavka]++;
    sem_post(bus_arrived_to_zastavka);

    // Wait untill bus arrives
    sem_wait(zastavka_sem[waits_on_zastavka - 1]);
    sem_wait(output);
    fprintf(fp, "%i: L %i: boarding\n", ++(*output_cnt), idL);
    sem_post(output);

    // Decrement number of skiers on stop
    zastavka_capacity[waits_on_zastavka]--;

    // Signal for bus that skier is boarded
    sem_post(boarding);

    // Wait untill bus arrives to final stop
    sem_wait(terminus);

    sem_wait(output);
    fprintf(fp, "%i: L %i: going to ski\n", ++(*output_cnt), idL);
    sem_post(output);

    (*transported_skiers)++;

    // Signal for bus that skier was dismounted
    sem_post(going_ski);

    if (fp != NULL)
        fclose(fp); // close file

    exit(EXIT_SUCCESS);
}

// Child process for skibus
int skibus()
{
    srand(time(NULL) * getpid()); // Initialization of randomizer

    int current_zastavka;
    int current_capacity;

    sem_wait(output);
    fprintf(fp, "%d: BUS: started\n", ++(*output_cnt));
    *transported_skiers = 0;
    sem_post(output);

    // While loop until all skiers are transported
    while (*transported_skiers < L)
    {
        current_capacity = 0;
        current_zastavka = 1;

        // Cycle for each stop
        for (; current_zastavka <= Z; current_zastavka++)
        {
            // Get random drive time
            usleep((rand() % TL + 1));

            // Sem wait to stop skiers from arriving to stop to protect critical section
            sem_wait(bus_arrived_to_zastavka);

            sem_wait(output);
            fprintf(fp, "%d: BUS: arrived to %i\n", ++(*output_cnt), current_zastavka);
            sem_post(output);

            // Cycle to board each skier on current stop until capacity is full or nobody is on stop
            while (current_capacity < K && (zastavka_capacity[current_zastavka]) != 0)
            {
                // Board one skier
                sem_post(zastavka_sem[current_zastavka - 1]);

                // Increment capacity
                current_capacity++;

                // Wait until skier is boarded
                sem_wait(boarding);
            }

            sem_wait(output);
            fprintf(fp, "%d: BUS: leaving %i\n", ++(*output_cnt), current_zastavka);
            sem_post(output);

            // Release ctitical section
            sem_post(bus_arrived_to_zastavka);
        }

        sem_wait(output);
        fprintf(fp, "%d: BUS: arrived to final\n", ++(*output_cnt));
        sem_post(output);

        // Dismount all skiers until capacity is 0
        while (current_capacity-- != 0)
        {
            // Dismount one skier
            sem_post(terminus);

            // Wait untill skier went to ski
            sem_wait(going_ski);
        }

        sem_wait(output);
        fprintf(fp, "%d: BUS: leaving final\n", ++(*output_cnt));
        sem_post(output);
    }

    // End
    sem_wait(output);
    fprintf(fp, "%d: BUS: finish\n", ++(*output_cnt));
    sem_post(output);

    if (fp != NULL)
        fclose(fp); // close file

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    // Arguments check
    if (argc != 6)
    {
        fprintf(stderr, "Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }
    L = atoi(argv[1]);
    Z = atoi(argv[2]);
    K = atoi(argv[3]);
    TL = atoi(argv[4]);
    TB = atoi(argv[5]);

    // Arguments format check
    if (L < 0 || (Z < 1 || Z > 10) || (K < 10 || K > 100) || (TL < 0 || TL > 10000) || (TB < 0 || TB > 1000))
    {
        fprintf(stderr, "Wrong format of arguments\n");
        exit(EXIT_FAILURE);
    }

    // Open file for output
    fp = fopen("proj2.out", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Open file for output error\n");
        exit(EXIT_FAILURE);
    }

    init_vars(Z);

    // Init skibus process
    pid_t skibus_process = fork();
    if (skibus_process == 0)
    {
        skibus();
    }
    else if (skibus_process < 0)
    {
        fprintf(stderr, "Failed to create skibus process \n");
        destroy_vars();
        if (fp != NULL)
        {
            fclose(fp);
        }
        // Killing  process
        kill(skibus_process, SIGKILL);

        exit(EXIT_FAILURE);
    }

    pid_t child_processes[L + 1]; // Array of skier processes
    for (int i = 1; i <= L; i++)
    {
        // Forking for number of lyzars
        child_processes[i - 1] = fork();
        if (child_processes[i - 1] == 0) // Case for child process
        {
            // Function to child process
            lyzars(i);
        }
        else if (child_processes[i - 1] < 0) // Case for error with forking
        {
            fprintf(stderr, "Failed to create process %d\n", i);
            destroy_vars();
            if (fp != NULL)
                fclose(fp);
            for (int j = 0; j < i; j++)
            {
                // Killing all child processes
                kill(child_processes[j], SIGKILL);
            }
            exit(EXIT_FAILURE);
        }
    }

    // while (wait(NULL) > 0)
    //     ;
    waitpid(skibus_process, NULL, 0);

    if (fp != NULL)
        fclose(fp); // close file

    destroy_vars();

    return 0;
}
