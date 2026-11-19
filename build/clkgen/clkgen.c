#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define	HDR_GUARD	"__BUILD_CLKGEN_CLOCK_CONF_HEADER__"

static long long g_max;

static void err_hdr(const char *path, const char *msg)
{
	FILE *f = fopen(path, "w");

	if (NULL == f)
		return;
	fprintf(f, "#ifndef " HDR_GUARD "\n");
	fprintf(f, "#error clkgen failed: %s\n", msg);
	fprintf(f, "#endif\n");
	fclose(f);
}

static int chip_max_hz(const char *dev_inc, const char *chip_defs,
	const char *series_uc, long long *out)
{
	char want[16];
	char macro[64];
	char path[512];
	char line[256];
	const char *q;
	size_t n = 0;
	FILE *f;

	snprintf(want, sizeof(want), "-DSTM32%s", series_uc);
	const char *p = strstr(chip_defs, want);
	if (NULL == p)
	{
		fprintf(stderr, "clkgen: device macro %s not found in CHIP_DEFS\n", want);
		return -1;
	}
	q = p;
	while (*q != '\0' && !isspace((unsigned char)*q) && n + 1 < sizeof(macro))
		macro[n++] = *q++;
	for (size_t i = 2; i < n; i++)
		macro[i - 2] = (char)tolower((unsigned char)macro[i]);
	macro[n - 2] = '\0';
	snprintf(path, sizeof(path), "%s/%s.h", dev_inc, macro);
	f = fopen(path, "r");
	if (NULL == f)
	{
		fprintf(stderr, "clkgen: cannot open %s\n", path);
		return -1;
	}
	while (NULL != fgets(line, sizeof(line), f))
	{
		long long v;

		if (1 == sscanf(line, "#define RCC_MAX_FREQUENCY %lld", &v))
		{
			fclose(f);
			*out = v;
			return 0;
		}
	}
	fclose(f);
	fprintf(stderr, "clkgen: RCC_MAX_FREQUENCY not found in %s\n", path);
	return -1;
}

#define	F4_PLL_IN_HZ	2000000LL
#define	F4_USB_HZ		48000000LL

static long long f4_def_sysclk(long long max)
{
	long long best = 0;

	for (long long q = 9; q >= 3; q--)
	{
		for (long long p = 2; p <= 8; p += 2)
		{
			long long v = F4_USB_HZ * q;
			long long s;

			if (0 != v % p)
				continue;
			s = v / p;
			if (s <= max && s > best)
				best = s;
		}
	}
	return best;
}

static long long f4_vco_hz(long long sys)
{
	for (long long p = 2; p <= 8; p += 2)
	{
		long long v = sys * p;

		if (0 == v % F4_USB_HZ && v / 1000000 >= 100 && v / 1000000 <= 432)
			return v;
	}
	return 0;
}

static int f4_gen(const char *dev_inc, const char *chip_defs, long long hse,
	const char *sys_arg, const char *out)
{
	long long sys;
	long long vco;
	long long m;
	long long n;
	long long p;
	long long q;
	FILE *f;

	if (0 != chip_max_hz(dev_inc, chip_defs, "F4", &g_max))
		return -1;
	if ('\0' == sys_arg[0] || '-' == sys_arg[0])
		sys = f4_def_sysclk(g_max);
	else
		sys = atoll(sys_arg);
	vco = f4_vco_hz(sys);
	if (0 == vco)
	{
		fprintf(stderr, "clkgen: SYSCLK_HZ %lld not representable: no P in {2,4,6,8} makes SYSCLK*P a 48M multiple in 100..432 MHz\n", sys);
		return -1;
	}
	m = hse / F4_PLL_IN_HZ;
	n = vco / F4_PLL_IN_HZ;
	p = vco / sys;
	q = vco / F4_USB_HZ;
	if (0 != hse % F4_PLL_IN_HZ)
	{
		fprintf(stderr, "clkgen: HSE_FREQ %lld not divisible by %lld\n", hse, F4_PLL_IN_HZ);
		return -1;
	}
	if (0 != vco % sys)
	{
		fprintf(stderr, "clkgen: VCO %lld not divisible by SYSCLK %lld\n", vco, sys);
		return -1;
	}
	if (sys > g_max)
	{
		fprintf(stderr, "clkgen: SYSCLK_HZ %lld exceeds chip max %lld\n", sys, g_max);
		return -1;
	}
	if (hse < 4000000 || hse > 26000000)
	{
		fprintf(stderr, "clkgen: HSE_FREQ %lld out of 4M..26M\n", hse);
		return -1;
	}
	if (2 != p && 4 != p && 6 != p && 8 != p)
	{
		fprintf(stderr, "clkgen: PLL_P %lld out of 2/4/6/8\n", p);
		return -1;
	}
	if (m < 2 || m > 63)
	{
		fprintf(stderr, "clkgen: PLL_M %lld out of 2..63\n", m);
		return -1;
	}
	if (n < 50 || n > 432)
	{
		fprintf(stderr, "clkgen: PLL_N %lld out of 50..432\n", n);
		return -1;
	}
	if (q < 2 || q > 15)
	{
		fprintf(stderr, "clkgen: PLL_Q %lld out of 2..15\n", q);
		return -1;
	}
	f = fopen(out, "w");
	if (NULL == f)
	{
		fprintf(stderr, "clkgen: cannot write %s\n", out);
		return -1;
	}
	fprintf(f, "#ifndef " HDR_GUARD "\n");
	fprintf(f, "#define " HDR_GUARD "\n");
	fprintf(f, "#define SYSCLK_HZ %lld\n", sys);
	fprintf(f, "#define SYSCLK_PLL_M %lld\n", m);
	fprintf(f, "#define SYSCLK_PLL_N %lld\n", n);
	fprintf(f, "#define SYSCLK_PLL_P %lld\n", p);
	fprintf(f, "#define SYSCLK_PLL_Q %lld\n", q);
	if (sys != g_max)
		fprintf(f, "#warning clkgen: chip max %lld not representable with 48M-anchored VCO, default %lld\n", g_max, sys);
	fprintf(f, "#endif\n");
	fclose(f);
	printf("  SYSCLK  %lld  HSE %lld  VCO %lld  PLL M=%lld N=%lld P=%lld Q=%lld", sys, hse, vco, m, n, p, q);
	if (sys != g_max)
		printf("  [chip max %lld]", g_max);
	printf("\n");
	return 0;
}

static int f1_gen(const char *dev_inc, const char *chip_defs, long long hse,
	const char *sys_arg, const char *out)
{
	long long sys;
	long long mul;
	FILE *f;

	if (0 != chip_max_hz(dev_inc, chip_defs, "F1", &g_max))
		return -1;
	if ('\0' == sys_arg[0] || '-' == sys_arg[0])
		sys = g_max;
	else
		sys = atoll(sys_arg);
	mul = sys / hse;
	if (0 != sys % hse)
	{
		fprintf(stderr, "clkgen: SYSCLK_HZ %lld not divisible by HSE_FREQ %lld\n", sys, hse);
		return -1;
	}
	if (sys > g_max)
	{
		fprintf(stderr, "clkgen: SYSCLK_HZ %lld exceeds chip max %lld\n", sys, g_max);
		return -1;
	}
	if (mul < 2 || mul > 16)
	{
		fprintf(stderr, "clkgen: PLL_MUL %lld out of 2..16\n", mul);
		return -1;
	}
	f = fopen(out, "w");
	if (NULL == f)
	{
		fprintf(stderr, "clkgen: cannot write %s\n", out);
		return -1;
	}
	fprintf(f, "#ifndef " HDR_GUARD "\n");
	fprintf(f, "#define " HDR_GUARD "\n");
	fprintf(f, "#define SYSCLK_HZ %lld\n", sys);
	fprintf(f, "#define SYSCLK_PLL_MUL %lld\n", mul);
	fprintf(f, "#endif\n");
	fclose(f);
	printf("  SYSCLK  %lld  HSE %lld  PLL MUL=%lld\n", sys, hse, mul);
	return 0;
}

int main(int argc, char **argv)
{
	if (7 != argc)
	{
		fprintf(stderr, "usage: clkgen <series> <dev_inc> <chip_defs> <hse_hz> <sysclk_hz|-> <out_hdr>\n");
		return 1;
	}
	if (0 == strcmp(argv[1], "f4"))
	{
		if (0 != f4_gen(argv[2], argv[3], atoll(argv[4]), argv[5], argv[6]))
		{
			err_hdr(argv[6], "see clkgen stderr above");
			return 1;
		}
		return 0;
	}
	if (0 == strcmp(argv[1], "f1"))
	{
		if (0 != f1_gen(argv[2], argv[3], atoll(argv[4]), argv[5], argv[6]))
		{
			err_hdr(argv[6], "see clkgen stderr above");
			return 1;
		}
		return 0;
	}
	fprintf(stderr, "clkgen: unknown series %s\n", argv[1]);
	return 1;
}
