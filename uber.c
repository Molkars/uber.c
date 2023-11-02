
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

// util

#define D_ARRAY_SIZE 10

typedef size_t usize_t;
typedef ssize_t isize_t;

#define TypeName(x) _Generic((x),                                                 	  \
            _Bool: "_Bool",                  unsigned char: "unsigned char",          \
             char: "char",                     signed char: "signed char",            \
        short int: "short int",         unsigned short int: "unsigned short int",     \
              int: "int",                     unsigned int: "unsigned int",           \
         long int: "long int",           unsigned long int: "unsigned long int",      \
    long long int: "long long int", unsigned long long int: "unsigned long long int", \
            float: "float",                         double: "double",                 \
      long double: "long double",                   char *: "pointer to char",        \
           void *: "pointer to void",                int *: "pointer to int",         \
          default: "other")

#define TypeFormatString(x) _Generic((x),					\
            _Bool: "%b",	unsigned char: "%c",			\
             char: "%c",	signed char: "%c",				\
        short int: "%hi",	unsigned short int: "%hu",		\
              int: "%d",	unsigned int: "%u",				\
         long int: "%ld",	unsigned long int: "%lu",		\
    long long int: "%lld",	unsigned long long int: "%llu",	\
            float: "%f",	double: "%lf",					\
      long double: "%Lf",	char *: "%s",					\
           void *: "%p",	int *: "%p",					\
		  void **: "%p",									\
          default: "")
	

#define __Panic(fmt, ...) do { \
	const char *prefix = "Panic in %s, line %d: "; \
	usize_t prefix_len = strlen(prefix); \
	char *fmt_str = fmt; \
	usize_t fmt_str_len = strlen(fmt_str); \
	char *out = calloc(prefix_len + fmt_str_len + 2, sizeof(char)); \
	if (out == NULL) { \
		printf("Panic in %s, line %d: ", __FILE__, __LINE__); \
	} else { \
		strcpy(out, prefix); \
		strcpy(out + prefix_len, fmt_str); \
		strcpy(out + prefix_len + fmt_str_len, "\n"); \
		printf(out, __FILE__, __LINE__, __VA_ARGS__); \
	} \
	exit(1); \
} while (0);

#define Panic(...) __Panic(__VA_ARGS__, 0)

#define Debug(v) printf(#v " = " TypeFormatString(v) "\n", v);

#define IsNull(v) (((const void *) v) == NULL)
#define NotNull(v) !IsNull(v)

#define Arg(s) do { \
	const void *__val = (const void *) s; \
	if (IsNull(__val)) { \
		Panic("%s: argument %s was null!", __FUNCTION__, #s); \
	} \
} while (0)

// collection

typedef struct vector_t {
	void *block; // the block goes first because we want to be able to use vector_t* as element_t*
	usize_t len, cap, size;
} vector_t;

void vector_init(vector_t *v, usize_t size) {
	Arg(v);
	if (size == 0) {
		Panic("vector_init called with zero-valued size!");
	}
	v->len = 0;
	v->cap = D_ARRAY_SIZE;
	v->size = size;
	v->block = calloc(D_ARRAY_SIZE, size);
	if (IsNull(v->block)) {
		Panic("vector_init was unable to allocate memory!");
	}
}

vector_t vector_create(usize_t size) {
	vector_t out;
	vector_init(&out, size);
	return out;
}

#define Vector(ty) vector_create(sizeof(ty));

usize_t vector_length(const struct vector_t *v) {
	Arg(v);
	return v->len;
}

usize_t vector_capacity(const struct vector_t *v) {
	Arg(v);
	return v->cap;
}

void vector_ensure_capacity(struct vector_t *v, size_t cap) {
	Arg(v);
	if (cap > v->cap) {
		v->cap = cap;
		v->block = realloc(v->block, cap * v->size);
		if (IsNull(v->block)) {
			Panic("vector_ensure_capacity was unable to reallocate buffer!");
		}
	}
}

void __vector_inbounds(const struct vector_t *vec, usize_t idx) {
	Arg(vec);
	if (idx < 0 || idx >= vec->len) {
		Panic("index %zu out of bounds for length %zu\n", idx, vec->len);
	}
}

void __vector_index_set(struct vector_t *vec, usize_t idx, void *elem) {
	Arg(vec);
	Arg(elem);
	__vector_inbounds(vec, idx);
	memcpy(
		((char *) vec->block) + idx * vec->size,
		elem,
		vec->size
	);
}

#define vector_index_set(vec, ty, idx, val) do { \
	ty __element = val; \
	__vector_index_set(vec, idx, &__element); \
} while (0);

void vector_push(struct vector_t *vec, void *elem) {
	Arg(vec);
	Arg(elem);
	vector_ensure_capacity(vec, vec->cap == 0 ? D_ARRAY_SIZE : vec->cap * 2);
	vec->len++;
	__vector_index_set(vec, vec->len - 1, elem);
}

#define vector_pushc(vec, ty, elem) do {	\
	ty __element = elem;		\
	vector_push(vec, &__element);	\
} while (0);

void *__vector_index(const struct vector_t *vec, usize_t idx) {
	Arg(vec);
	__vector_inbounds(vec, idx);
	return (void *) (((char *) vec->block) + idx * vec->size);
}

#define vector_index(vec, ty, idx) *((ty *) __vector_index(vec, idx))

void vector_remove(struct vector_t *vec, usize_t idx) {
	Arg(vec);
	__vector_inbounds(vec, idx);
	char *head = ((char *) vec->block) + idx * vec->size;
	memcpy(head, head + vec->size, vec->len - idx);
	vec->len--;
}

void vector_swap_remove(struct vector_t *vec, usize_t idx) {
	Arg(vec);
	__vector_inbounds(vec, idx);
	if (vec->len == 1) {
		vec->len = 0;
	} else {
		void *last = __vector_index(vec, vec->len - 1);
		void *elem = __vector_index(vec, idx);
		memcpy(elem, last, vec->size);
		vec->len--;
	}
}

// struct ustring_t {
// 	struct vector_t inner;
// };
// 
// void ustring_init(struct ustring *s) {
// 	Arg(s);
// 	
// }
// void ustring_concat(struct ustring *s, void *other) {}
// char *ustring_c(const struct ustring *s) {
// return NULL;
// }


// programs
// ========

int main(int argc, char **argv) {
	vector_t vec = Vector(int);	

	return 0;
}

