#ifndef JSON_INPUT_H_INCLUDED
#define JSON_INPUT_H_INCLUDED

#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable: 4200)
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct jsi_dialect {
	unsigned allow_trailing_comma : 1;
	unsigned allow_missing_comma : 1;
	unsigned allow_bare_keys : 1;
	unsigned allow_comments : 1;
	unsigned allow_unknown_escape : 1;
	unsigned allow_control_in_string : 1;
} jsi_dialect;

typedef struct jsi_error {
	const char *description;
	size_t line, column;
	size_t byte_offset;
} jsi_error;

typedef void *(*jsi_alloc_fn)(void *user, size_t size);
typedef void *(*jsi_realloc_fn)(void *user, void *ptr, size_t new_size, size_t old_size);
typedef void (*jsi_free_fn)(void *user, void *ptr, size_t size);

typedef struct jsi_allocator {
	jsi_alloc_fn alloc_fn;
	jsi_realloc_fn realloc_fn;
	jsi_free_fn free_fn;
	void *user;

	size_t memory_used;
	size_t memory_limit;
} jsi_allocator;

typedef struct jsi_args {

	// Output
	jsi_error error;
	size_t result_used;
	size_t end_offset;

	// Input
	void *result_buffer;
	size_t result_size;

	void *temp_buffer;
	size_t temp_size;

	jsi_allocator result_allocator;
	jsi_allocator temp_allocator;

	int nesting_limit;

	unsigned allow_trailing_data : 1;
	unsigned no_allocation : 1;
	unsigned implicit_root_object : 1;
	unsigned implicit_root_array : 1;
	jsi_dialect dialect;

} jsi_args;

typedef enum jsi_type {
	jsi_type_undefined,
	jsi_type_null,
	jsi_type_boolean,
	jsi_type_number,
	jsi_type_string,
	jsi_type_object,
	jsi_type_array,
} jsi_type;

typedef struct jsi_obj jsi_obj;
typedef struct jsi_arr jsi_arr;
typedef struct jsi_obj_map jsi_obj_map;

typedef struct jsi_value {
	jsi_type type;
	uint16_t key_hash;
	union {
		int boolean;
		double number;
		const char *string;
		jsi_obj *object;
		jsi_arr *array;
	};
} jsi_value;

typedef struct jsi_prop {
	const char *key;
	jsi_value value;
} jsi_prop;

struct jsi_obj {
	jsi_obj_map *map;

	size_t num_props;
	jsi_prop props[];
};

struct jsi_arr {
	size_t num_values;
	jsi_value values[];
};

typedef const void *(*jsi_refill_fn)(void *user, size_t *size);

jsi_value *jsi_parse_memory(const void *data, size_t size, jsi_args *args);
jsi_value *jsi_parse_string(const char *str, jsi_args *args);
jsi_value *jsi_parse_file(const char *filename, jsi_args *args);
jsi_value *jsi_parse_stream(jsi_refill_fn refill, void *user, jsi_args *args);
jsi_value *jsi_parse_stream_initial(jsi_refill_fn refill, void *user, const void *data, size_t size, jsi_args *args);

void jsi_free(jsi_value *value);

jsi_value *jsi_get_len(jsi_obj *obj, const char *key, size_t length);
static jsi_value *jsi_get(jsi_obj *obj, const char *key) {
	return jsi_get_len(obj, key, strlen(key));
}

static size_t jsi_length(const char *jsi_str) {
	return (size_t)((const uint32_t*)jsi_str)[-1];
}
static size_t jsi_equal_len(const char *jsi_str, const char *str, size_t length) {
	return (size_t)((const uint32_t*)jsi_str)[-1] == length && !memcmp(jsi_str, str, length);
}
static size_t jsi_equal(const char *jsi_str, const char *str) {
	size_t length = strlen(str);
	return (size_t)((const uint32_t*)jsi_str)[-1] == length && !memcmp(jsi_str, str, length);
}

#ifdef __cplusplus
	static jsi_prop *begin(jsi_obj &obj) { return obj.props; }
	static jsi_prop *end(jsi_obj &obj) { return obj.props + obj.num_props; }
	static jsi_value *begin(jsi_arr &arr) { return arr.values; }
	static jsi_value *end(jsi_arr &arr) { return arr.values + arr.num_values; }
#endif

#if defined(_MSC_VER)
	#pragma warning(pop)
#endif

#endif
