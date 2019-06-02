// key = string to match or null
// klen = key length (or 0), or if null key then len is the array offset value
// json = json object or array
// jlen = length of json
// vlen = where to store return value length
// returns pointer to value and sets len to value length, or 0 if not found
// any parse error will set vlen to the position of the error
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct status
	{
		const char *json;
		size_t jlen;

		const char *cur, *end, *start;
		void **go;
		int depth;
	} status_t;

	void js0n_init(status_t *s, const char *json, size_t jlen, size_t offset);
	const char *js0n_run(status_t *s, const char *key, size_t klen, size_t *vlen);

#ifdef __cplusplus
} /* extern "C" */
#endif
