// http://www.ganssle.com/item/automatically-debugging-firmware.htm
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <assert.h>

static void handle_fpe(int sig)
{
	printf("Got SIGFPE! sig=%d\n", sig);
}

float solve_post(float a, float b, float c)
{
	float result;

	result = (-b + sqrt(b * b - 4 * a * c)) / 2 * a;
	return result;
}

float solve_post_with_asserts(float a, float b, float c)
{
	float result;

	assert(a != 0.0);
	assert((b*b - 4*a*c) >= 0.0);
	result = (-b + sqrt(b * b - 4 * a * c)) / 2 * a;
	assert(isfinite(result));

	return result;
}

main()
{
	signal(SIGFPE, handle_fpe);

	printf("res = %.32f\n", solve_post(1.0, 10.0, 3.0));
	/* We're now going to divide by 0!
	 * No compiler warning, no runtime exception, nothing !
	 * (why don't we get SIGFPE ??)
	 */
	printf("div by 0 no asserts:   res = %.32f\n", solve_post(0.0, 10.0, 3.0));
	printf("div by 0 with asserts: res = %.32f\n", solve_post_with_asserts(0.0, 10.0, 3.0));
/*	printf("v small 'a' val with asserts: res = %.32f\n",
					solve_post_with_asserts(0.0000000000000000000000000000000001, 10.0, 3.0));
	*/
}
