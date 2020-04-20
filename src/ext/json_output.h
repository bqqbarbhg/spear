#pragma once

#include <string.h>
#include <stdbool.h>

#define JSO_BUFFER_MIN_SIZE 64
#define JSO_MAX_INDENTS 16

// Input

typedef struct jso_stream jso_stream;
typedef void jso_flush_fn(jso_stream *s);
typedef void jso_close_fn(jso_stream *s);

struct jso_stream {
	char *data;
	size_t capacity;
	size_t pos;

	jso_flush_fn *flush_fn;
	jso_close_fn *close_fn;
	void *user;

	bool add_comma;
	bool failed;

	int level;
	int single_line_level;
	bool pretty_open;
	bool pretty_array;

	// Configuration
	bool pretty;
	bool trailing_comma;
};

void jso_init_custom(jso_stream *s);
void jso_init_memory(jso_stream *s, void *dst, size_t size);
void jso_init_growable(jso_stream *s);
int jso_init_file(jso_stream *s, const char *filename);
int jso_init_file_buf(jso_stream *s, const char *filename, void *buffer, size_t size);
int jso_close(jso_stream *s);
void jso_flush(jso_stream *s);

void jso_null(jso_stream *s);
void jso_boolean(jso_stream *s, int value);
void jso_int(jso_stream *s, int value);
void jso_uint(jso_stream *s, unsigned value);
void jso_double(jso_stream *s, double value);
void jso_string(jso_stream *s, const char *value);
void jso_string_len(jso_stream *s, const char *value, size_t length);
void jso_json(jso_stream *s, const char *json);
void jso_json_len(jso_stream *s, const char *json, size_t length);

void jso_object(jso_stream *s);
void jso_array(jso_stream *s);
void jso_end_object(jso_stream *s);
void jso_end_array(jso_stream *s);

void jso_prop(jso_stream *s, const char *key);
void jso_prop_len(jso_stream *s, const char *key, size_t length);

void jso_single_line(jso_stream *s);

static void jso_prop_null(jso_stream *s, const char *key) { jso_prop(s, key); jso_null(s); }
static void jso_prop_boolean(jso_stream *s, const char *key, int value) { jso_prop(s, key); jso_boolean(s, value); }
static void jso_prop_int(jso_stream *s, const char *key, int value) { jso_prop(s, key); jso_int(s, value); }
static void jso_prop_double(jso_stream *s, const char *key, double value) { jso_prop(s, key); jso_double(s, value); }
static void jso_prop_string(jso_stream *s, const char *key, const char *value) { jso_prop(s, key); jso_string(s, value); }
static void jso_prop_string_len(jso_stream *s, const char *key, const char *value, size_t length) { jso_prop(s, key); jso_string_len(s, value, length); }
static void jso_prop_object(jso_stream *s, const char *key) { jso_prop(s, key); jso_object(s); }
static void jso_prop_array(jso_stream *s, const char *key) { jso_prop(s, key); jso_array(s); }
static void jso_prop_json(jso_stream *s, const char *key, const char *json) { jso_prop(s, key); jso_json(s, json); }
static void jso_prop_json_len(jso_stream *s, const char *key, const char *json, size_t length) { jso_prop(s, key); jso_json_len(s, json, length); }

