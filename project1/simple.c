#include <stdio.h>
#include<math.h>

int main() {
    int n=10, i=0;
    float num[10]= {1.0, 1.0, 1.0, 7.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.9};
    float sum = 0.0, avg;
    for (i = 0; i < n; ++i) {
        sum += sqrt(num[i]);
    }
    avg = sum / n;
    printf("Average = %.2f\n", avg);
    return 0;
}
