/*
 * 1by10.c
 * Dividing 1.0 by 10.0 (floating point) shows the result:
 *  0.100000000000000005551115123126
 * clearly demonstrating how the number of digits of precision matters!
 * (Here, the 55111... portion begins after 18 digits! If the register holding
 * the result is smaller, it gets truncated. With something like a missile
 * tracking system, the truncation can be literally fatal!).
 */
#include <stdio.h>
double res = 0;

int main()
{
	res = 1.0/10.0;
	printf("res of 1/10 is %.30f\n", res);
}
