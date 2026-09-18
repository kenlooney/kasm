#include <stdio.h>
extern int answer(void);
int main(void)
{
    int result = answer();
    printf("answer = %d\n", result);
    return result == 42 ? 0 : 1;
}
