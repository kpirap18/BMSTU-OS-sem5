#include <stdio.h>

#define N 7

void sort_buble(int *mass,  int n)
{
    int no_swap = 0;
    int tmp;
    
    for (int i = n - 1; i >= 0; i--)
    {
        no_swap = 1;
        for (int j = 0; j < i; j++)
        {
            if (mass[j] > mass[j + 1])
            {
                tmp = mass[j];
                mass[j] = mass[j + 1];
                mass[j + 1] = tmp;
                no_swap = 0;
            }
        }
        if (no_swap == 1)
            break;
    }
}

void main()
{
    int mass[N] = {4, 9, 2, -1, 8, 3, 5};
    
    printf("\n proc 1 (sort array) START\n");
    printf("Arrat before: ");
    for (int i = 0; i < N; i++)
    {
        printf("%d ", mass[i]);
    }
    printf("\n");

    sort_buble(mass, N);

    printf("Array after: ");
    for (int i = 0; i < N; i++)
    {
        printf("%d ", mass[i]);
    }
    printf("\n proc 1 (sort array) END\n\n");
}
