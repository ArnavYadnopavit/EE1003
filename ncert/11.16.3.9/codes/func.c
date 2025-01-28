#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void simulate_bernoulli(double p_a, int n_simulations, double *empirical_pmf) {
    int count_0 = 0, count_1 = 0; // Counters for outcomes 0 and 1

    // Seed the random number generator
    srand(time(NULL));

    // Simulate Bernoulli trials
    for (int i = 0; i < n_simulations; i++) {
        double rand_num = (double)rand() / RAND_MAX; // Generate a random number in [0, 1)
        if (rand_num < p_a) {
            count_1++; // Outcome 1 (success)
        } else {
            count_0++; // Outcome 0 (failure)
        }
    }

    // Calculate the empirical PMF
    empirical_pmf[0] = (double)count_0 / n_simulations; // Probability of 0 (not A)
    empirical_pmf[1] = (double)count_1 / n_simulations; // Probability of 1 (A)
}

int main() {
    double p_a = 1.0 / 12.0; // Probability of event A (success)
    int n_simulations = 10000; // Number of simulations
    double empirical_pmf[2]; // Array to store probabilities of 0 and 1

    // Call the simulation function
    simulate_bernoulli(p_a, n_simulations, empirical_pmf);

    // Print the results
    printf("Empirical PMF:\n");
    printf("P(X = 0): %.5f\n", empirical_pmf[0]); // Probability of "not A"
    printf("P(X = 1): %.5f\n", empirical_pmf[1]); // Probability of "A"

    return 0;
}

