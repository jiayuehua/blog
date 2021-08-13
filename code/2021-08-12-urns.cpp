// g++ -std=c++11 urns.cpp -o urns
// ./urns > listen.txt
// ./urns x > deaf.txt
// And then run this Python program:
#if 0
  import matplotlib.pyplot as plt
  import numpy as np
  l = [int(line) for line in open('listen.txt').readlines()]
  d = [int(line) for line in open('deaf.txt').readlines()]
  _, axs = plt.subplots(2)
  axs[0].hist(d, bins=[x-0.5 for x in range(102)], weights=np.ones(len(d)) / len(d))
  axs[1].hist(l, bins=[x-0.5 for x in range(102)], weights=np.ones(len(d)) / len(d))
  plt.show()
#endif

#include <stdio.h>
#include <random>

std::mt19937 g;

// Without loss of generality, assume that the red urn was picked.
// So anyone who guesses "red" is correct, and anyone who guesses
// "blue" is incorrect.

int listening_strategy(int num_players)
{
    for (int i=0; i < num_players/2; ++i) {
        bool p0_guessed_red = ((g() % 3) != 0);
        bool p1_guessed_red = ((g() % 3) != 0);

        if (p0_guessed_red == p1_guessed_red) {
            // everyone else wins or loses together
            return i + (p0_guessed_red ? (num_players - 2*i) : 0);
        }
    }
    return 50;
}

int deaf_strategy(int num_players)
{
    int count = 0;
    for (int i=0; i < num_players; ++i) {
        count += ((g() % 3) != 0);
    }
    return count;
}

int main(int argc, char **)
{
    if (argc == 1) {
        for (int i=0; i < 100000; ++i) {
            printf("%d\n", listening_strategy(100));
        }
    } else {
        for (int i=0; i < 100000; ++i) {
            printf("%d\n", deaf_strategy(100));
        }
    }
}
