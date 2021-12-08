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

void read_array(int *array, int n)
{
    printf("P1: Input array (len = %d): ", n);
    for (int i = 0; i < n; i++)
    {
        int a;
        if (scanf("%d", &a) != 1)
        {
            printf("Error, input only integer!\n");
        }
        else
        {
            array[i] = a;
        }
    }
    printf("\n");
}

void main()
{
    // int mass[N] = {4, 9, 2, -1, 8, 3, 5};
    int mass[N];

    printf("\n !proc 1 (sort array) START\n");
    read_array(mass, N);

    printf("Array before: ");
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
    printf("\n !proc 1 (sort array) END\n\n");
}
