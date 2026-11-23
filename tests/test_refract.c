/* Unit test for Snell's law refraction & Total Internal Reflection (TIR). */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int	snell(const float d[3], const float n[3], float ior, float out[3])
{
	float	cosi = d[0] * n[0] + d[1] * n[1] + d[2] * n[2];
	float	eta = 1.0f / ior;
	float	nn[3] = {n[0], n[1], n[2]};
	float	k;
	float	len;

	if (cosi < 0.0f)
		cosi = -cosi;
	else
	{
		eta = ior;
		nn[0] = -n[0];
		nn[1] = -n[1];
		nn[2] = -n[2];
	}
	k = 1.0f - eta * eta * (1.0f - cosi * cosi);
	if (k < 0.0f)
		return (0);
	out[0] = d[0] * eta + nn[0] * (eta * cosi - sqrtf(k));
	out[1] = d[1] * eta + nn[1] * (eta * cosi - sqrtf(k));
	out[2] = d[2] * eta + nn[2] * (eta * cosi - sqrtf(k));
	len = sqrtf(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
	out[0] /= len;
	out[1] /= len;
	out[2] /= len;
	return (1);
}

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
	float	out[3];
	float	d_norm[3] = {0, 0, -1};
	float	n_up[3] = {0, 0, 1};
	float	d_45[3] = {0.70710678f, 0, -0.70710678f};
	float	d_tir[3] = {0.8660254f, 0, 0.5f};

	expect(snell(d_norm, n_up, 1.5f, out) && fabsf(out[2] + 1.0f) < 1e-4f,
		"normal incidence straight through");
	expect(snell(d_45, n_up, 1.5f, out)
		&& fabsf(out[0] - (0.70710678f / 1.5f)) < 1e-4f,
		"Snell sin(t2) = sin(t1)/ior");
	expect(!snell(d_tir, n_up, 1.5f, out),
		"TIR triggered at 60 deg glass->air");
	printf("all refract tests passed\n");
	return (0);
}
