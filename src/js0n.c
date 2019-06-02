// by jeremie miller - 2014
// public domain or MIT license, contributions/improvements welcome via github at https://github.com/quartzjer/js0n

#include <string.h> // one strncmp() is used to do key comparison, and a strlen(key) if no len passed in
#include "./js0n.h"
#include <stdio.h>

// gcc started warning for the init syntax used here, is not helpful so don't generate the spam, supressing the warning is really inconsistently supported across versions
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Winitializer-overrides"
#pragma GCC diagnostic ignored "-Woverride-init"

// only at depth 1, track start pointers to match key/value
#define PUSH(i)                        \
	if (depth == 1)                    \
	{                                  \
		if (!index)                    \
		{                              \
			val = s->cur + i;          \
		}                              \
		else                           \
		{                              \
			if (klen && index == 1)    \
				s->start = s->cur + i; \
			else                       \
				index--;               \
		}                              \
	}

// determine if key matches or value is complete
#define CAP(i)                                                                                          \
	if (depth == 1)                                                                                     \
	{                                                                                                   \
		if (val && !index)                                                                              \
		{                                                                                               \
			*vlen = (size_t)((s->cur + i + 1) - val);                                                   \
			s->go = go;                                                                                 \
			s->depth = depth;                                                                           \
			if (i == 0)                                                                                 \
				s->cur++;                                                                               \
			return val;                                                                                 \
		};                                                                                              \
		if (klen && s->start)                                                                           \
		{                                                                                               \
			index = (klen == (size_t)(s->cur - s->start) && strncmp(key, s->start, klen) == 0) ? 0 : 2; \
			s->start = 0;                                                                               \
		}                                                                                               \
	}

void js0n_init(status_t *s, const char *json, size_t jlen, size_t offset)
{
	s->json = json;
	s->jlen = jlen;
	s->start = json + offset;
	s->cur = json + offset;
	s->end = json + jlen;
	s->go = 0;
	s->depth = 0;
}

// this makes a single pass across the json bytes, using each byte as an index into a jump table to build an index and transition state
const char *js0n_run(status_t *s, const char *key, size_t klen, size_t *vlen)
{
	const char *val = 0;
	int depth = s->depth;
	size_t index = 1;
	static void *gostruct[] =
		{
			[0 ... 255] = &&l_bad,
			['\t'] = &&l_loop,
			[' '] = &&l_loop,
			['\r'] = &&l_loop,
			['\n'] = &&l_loop,
			['"'] = &&l_qup,
			[':'] = &&l_loop,
			[','] = &&l_loop,
			['['] = &&l_up,
			[']'] = &&l_down, // tracking [] and {} individually would allow fuller validation but is really messy
			['{'] = &&l_up,
			['}'] = &&l_down,
			['-'] = &&l_bare,
			[48 ... 57] = &&l_bare, // 0-9
			[65 ... 90] = &&l_bare, // A-Z
			[97 ... 122] = &&l_bare // a-z
		};
	static void *gobare[] =
		{
			[0 ... 31] = &&l_bad,
			[32 ... 126] = &&l_loop, // could be more pedantic/validation-checking
			['\t'] = &&l_unbare,
			[' '] = &&l_unbare,
			['\r'] = &&l_unbare,
			['\n'] = &&l_unbare,
			[','] = &&l_unbare,
			[']'] = &&l_unbare,
			['}'] = &&l_unbare,
			[':'] = &&l_unbare,
			[127 ... 255] = &&l_bad};
	static void *gostring[] =
		{
			[0 ... 127] = &&l_loop,
			['\\'] = &&l_esc,
			['"'] = &&l_qdown,
			[128 ... 255] = &&l_loop};
	static void *goesc[] =
		{
			[0 ... 255] = &&l_bad,
			['"'] = &&l_unesc,
			['\\'] = &&l_unesc,
			['/'] = &&l_unesc,
			['b'] = &&l_unesc,
			['f'] = &&l_unesc,
			['n'] = &&l_unesc,
			['r'] = &&l_unesc,
			['t'] = &&l_unesc,
			['u'] = &&l_unesc};
	void **go = s->go;
	if (!go)
	{
		go = gostruct;
	}

	if (!s->json || s->jlen <= 0)
	{
		return 0;
	}

	// no key is array mode, klen provides requested index
	if (!key)
	{
		index = klen;
		klen = 0;
	}
	else
	{
		if (klen <= 0)
		{
			klen = strlen(key); // convenience
		}
	}

	//printf("len =%d\n", s->jlen);
	for (; s->cur < s->end; s->cur++)
	{
		//printf("%c", *s->cur);
		goto *go[(unsigned char)*s->cur];
	l_loop:;
	}

	if (depth)
		*vlen = s->jlen; // incomplete
	s->depth = depth;
	return 0;

l_bad:
	//printf("badbad=%d", *(unsigned char *)s->cur);
	*vlen = s->cur - s->json; // where error'd
	s->depth = depth;
	return 0;

l_up:
	PUSH(0);
	++depth;
	goto l_loop;

l_down:
	--depth;
	CAP(0);
	goto l_loop;

l_qup:
	PUSH(1);
	go = gostring;
	goto l_loop;

l_qdown:
	CAP(-1);
	go = gostruct;
	goto l_loop;

l_esc:
	go = goesc;
	goto l_loop;

l_unesc:
	go = gostring;
	goto l_loop;

l_bare:
	PUSH(0);
	go = gobare;
	goto l_loop;

l_unbare:
	CAP(-1);
	go = gostruct;
	goto *go[(unsigned char)*s->cur];
}

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#endif