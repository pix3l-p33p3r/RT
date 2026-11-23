/* Unit test for mirror reflection direction & recursive blend math. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void	expect(int ok, const char *msg)
{
	if (!ok)
	{
		fprintf(stderr, "FAIL: %s\n", msg);
		exit(1);
	}
	printf("ok: %s\n", msg);
}

int	main(void)
{
	float	d[3] = {0.70710678f, 0.0f, -0.70710678f};
	float	n[3] = {0.0f, 0.0f, 1.0f};
	float	dot = d[0] * n[0] + d[1] * n[1] + d[2] * n[2];
	float	r[3] = {d[0] - 2 * dot * n[0], d[1] - 2 * dot * n[1],
			d[2] - 2 * dot * n[2]};
	float	blend;

	expect(fabsf(r[0] - 0.70710678f) < 1e-4f, "refl x preserved");
	expect(fabsf(r[2] - 0.70710678f) < 1e-4f, "refl z flipped +");
	blend = (1.0f - 0.8f) * 1.0f + 0.8f * 0.25f;
	expect(fabsf(blend - 0.4f) < 1e-5f, "energy blend 80% mirror");
	printf("all reflect tests passed\n");
	return (0);
}
