#include <stdio.h>

int main() {
    int i = 0;
    for (int z = 1; true; ++z) {
        for (int x = 1; x < z; ++x) {
            for (int y = x; y < z; ++y) {
                if (x*x + y*y == z*z) {
                    printf("(%d,%d,%d)\n", x, y, z);
                    if (++i == 10) goto done;
                }
            }
        }
    }
done:;
}
