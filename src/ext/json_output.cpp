#define _CRT_SECURE_NO_WARNINGS

#include "json_output.h"

#include <stdlib.h>
#include <stdio.h>

// Output

static char jso_g_shared_fail_memory[JSO_BUFFER_MIN_SIZE];

void jso_init_custom(jso_stream *s)
{
	memset(s, 0, sizeof(jso_stream));
}

static void jso_fn_memory_flush(jso_stream *s)
{
	// If memory writing fails set the write buffer to a shared array.
	// This seems like a race condition but since the data is never read
	// it's only a harmless data race.
	if (s->pos < s->capacity) {
		s->data[s->pos] = '\0';
	} else if (s->capacity > 0) {
		s->data[s->capacity - 1] = '\0';
	}
	s->data = jso_g_shared_fail_memory;
	s->capacity = sizeof(jso_g_shared_fail_memory);
	s->pos = 0;
	s->failed = 1;
}

static void jso_fn_memory_close(jso_stream *s)
{
	if (s->pos < s->capacity) {
		s->data[s->pos] = '\0';
	} else {
		if (s->capacity > 0)
			s->data[s->capacity - 1] = '\0';
		s->failed = 1;
	}
}

void jso_init_memory(jso_stream *s, void *dst, size_t size)
{
	jso_init_custom(s);
	s->data = (char*)dst;
	s->capacity = size;
	s->user = NULL;
	s->flush_fn = &jso_fn_memory_flush;
	s->close_fn = &jso_fn_memory_close;
	if (size < JSO_BUFFER_MIN_SIZE) {
		s->failed = 1;
		if (size) ((char*)dst)[0] = '\0';
		s->data = jso_g_shared_fail_memory;
		s->capacity = sizeof(jso_g_shared_fail_memory);
	}
}

static void jso_fn_growable_flush(jso_stream *s)
{
	s->capacity *= 2;
	s->data = (char*)realloc(s->data, s->capacity);
	s->pos = 0;
}

static void jso_fn_growable_close(jso_stream *s)
{
	free(s->data);
}

void jso_init_growable(jso_stream *s)
{
	jso_init_custom(s);
	s->capacity = 4096;
	s->data = (char*)malloc(s->capacity);
	s->flush_fn = &jso_fn_growable_flush;
	s->close_fn = &jso_fn_growable_close;
}

static void jso_fn_file_flush(jso_stream *s)
{
	fwrite(s->data, 1, s->pos, (FILE*)s->user);
	s->pos = 0;
}

static void jso_fn_file_close(jso_stream *s)
{
	fwrite(s->data, 1, s->pos, (FILE*)s->user);
	fclose((FILE*)s->user);
	free(s->data);
}

static void jso_fn_file_buf_close(jso_stream *s)
{
	fwrite(s->data, 1, s->pos, (FILE*)s->user);
	fclose((FILE*)s->user);
}

int jso_init_file(jso_stream *s, const char *filename)
{
	jso_init_custom(s);
	s->failed = 1;
	FILE *file = fopen(filename, "wb");
	if (!file) return 0;
	s->capacity = 4096;
	s->data = (char*)malloc(s->capacity);
	s->user = file;
	s->flush_fn = &jso_fn_file_flush;
	s->close_fn = &jso_fn_file_close;
	s->failed = 0;
	return 1;
}

int jso_init_file_buf(jso_stream *s, const char *filename, void *buffer, size_t size)
{
	jso_init_custom(s);
	s->failed = 1;
	if (size < JSO_BUFFER_MIN_SIZE) return 0;
	FILE *file = fopen(filename, "wb");
	if (!file) return 0;
	s->data = (char*)buffer;
	s->capacity = size;
	s->user = file;
	s->flush_fn = &jso_fn_file_flush;
	s->close_fn = &jso_fn_file_buf_close;
	s->failed = 0;
	return 1;
}

int jso_close(jso_stream *s)
{
	s->close_fn(s);
	return !s->failed;
}

void jso_flush(jso_stream *s)
{
	s->flush_fn(s);
}

static void jso_indent(jso_stream *s)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	s->data[s->pos++] = '\n';

	for (int i = s->level; i > 0; --i) {
		if (s->capacity - s->pos < 2) s->flush_fn(s);
		s->data[s->pos++] = ' ';
		s->data[s->pos++] = ' ';
	}
}

static void jso_prettify(jso_stream *s)
{
	if (s->add_comma || s->pretty_open) {
		if (s->single_line_level > 0 && s->level >= s->single_line_level) {
			if (s->add_comma || !s->pretty_array) {
				if (s->capacity == s->pos) s->flush_fn(s);
				s->data[s->pos++] = ' ';
			}
		} else {
			jso_indent(s);
		}
		s->pretty_open = false;
	}
}

void jso_boolean(jso_stream *s, int value)
{
	if (s->capacity - s->pos < 6) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	if (value) {
		memcpy(s->data + s->pos, "true", 4);
		s->pos += 4;
	} else {
		memcpy(s->data + s->pos, "false", 5);
		s->pos += 5;
	}
}

void jso_int(jso_stream *s, int value)
{
	if (s->capacity - s->pos < JSO_BUFFER_MIN_SIZE) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	s->pos += snprintf(s->data + s->pos, s->capacity - s->pos, "%d", value);
}

void jso_uint(jso_stream *s, unsigned value)
{
	if (s->capacity - s->pos < JSO_BUFFER_MIN_SIZE) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	s->pos += snprintf(s->data + s->pos, s->capacity - s->pos, "%u", value);
}

void jso_double(jso_stream *s, double value)
{
	if (s->capacity - s->pos < JSO_BUFFER_MIN_SIZE) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	s->pos += snprintf(s->data + s->pos, s->capacity - s->pos, "%f", value);
}

static void jso_raw_string(jso_stream *s, const char *value)
{
	const char *ptr = value;
	char c;
	if (s->pos - s->capacity < 4) s->flush_fn(s);
	s->data[s->pos++] = '"';
	while ((c = *ptr++) != '\0') {
		if (s->capacity - s->pos < 4) s->flush_fn(s);
		if (c == '\\' || c == '"' || c == '\n' || c == '\r' || c == '\t' || c == '\f') {
			char escape = c;
			switch (c) {
			case '\n': escape = 'n'; break;
			case '\r': escape = 'r'; break;
			case '\t': escape = 't'; break;
			case '\f': escape = 'f'; break;
			}
			s->data[s->pos + 0] = '\\';
			s->data[s->pos + 1] = escape;
			s->pos += 2;
		} else {
			s->data[s->pos++] = c;
		}
	}
	s->data[s->pos++] = '"';
}

static void jso_raw_string_len(jso_stream *s, const char *value, size_t length)
{
	const char *ptr = value, *end = ptr + length;
	if (s->pos - s->capacity < 4) s->flush_fn(s);
	s->data[s->pos++] = '"';
	for (; ptr != end; ptr++) {
		char c = *ptr;
		if (s->capacity - s->pos < 4) s->flush_fn(s);
		if (c == '\\' || c == '"' || c == '\n' || c == '\r' || c == '\t' || c == '\f') {
			char escape = c;
			switch (c) {
			case '\n': escape = 'n'; break;
			case '\r': escape = 'r'; break;
			case '\t': escape = 't'; break;
			case '\f': escape = 'f'; break;
			}
			s->data[s->pos + 0] = '\\';
			s->data[s->pos + 1] = escape;
			s->pos += 2;
		} else {
			s->data[s->pos++] = c;
		}
	}
	s->data[s->pos++] = '"';
}

void jso_string(jso_stream *s, const char *value)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	jso_raw_string(s, value);
}

void jso_string_len(jso_stream *s, const char *value, size_t length)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	jso_raw_string_len(s, value, length);
}

void jso_null(jso_stream *s)
{
	if (s->capacity - s->pos < 5) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	memcpy(s->data + s->pos, "null", 4);
	s->pos += 4;
}

void jso_json(jso_stream *s, const char *json)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	size_t length = strlen(json);
	size_t pos = 0;
	size_t space = s->capacity - s->pos;
	while (space < length) {
		memcpy(s->data + s->pos, json + pos, space);
		s->pos = s->capacity;
		s->flush_fn(s);
	}
	memcpy(s->data + s->pos, json + pos, length - pos);
	s->pos += length - pos;
}

void jso_json_len(jso_stream *s, const char *json, size_t length)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->add_comma = 1;
	size_t pos = 0;
	size_t space = s->capacity - s->pos;
	while (space < length) {
		memcpy(s->data + s->pos, json + pos, space);
		s->pos = s->capacity;
		s->flush_fn(s);
	}
	memcpy(s->data + s->pos, json + pos, length - pos);
	s->pos += length - pos;
}

static void jso_prettify_begin_array(jso_stream *s)
{
	s->pretty_open = true;
	s->pretty_array = true;
	s->level++;
}

static void jso_prettify_begin_object(jso_stream *s)
{
	s->pretty_open = true;
	s->pretty_array = false;
	s->level++;
}

static void jso_prettify_end_array(jso_stream *s)
{
	s->pretty_open = false;
	s->level--;
	if (s->level + 1 == s->single_line_level) {
		s->single_line_level = 0;
	} else {
		if (s->add_comma) {
			if (s->trailing_comma) {
				if (s->pos == s->capacity) s->flush_fn(s);
				s->data[s->pos++] = ',';
			}
			jso_indent(s);
		}
	}
}

static void jso_prettify_end_object(jso_stream *s)
{
	s->pretty_open = false;
	s->level--;
	if (s->level + 1 == s->single_line_level) {
		s->single_line_level = 0;
		if (s->add_comma) {
			if (s->pos == s->capacity) s->flush_fn(s);
			s->data[s->pos++] = ' ';
		}
	} else {
		if (s->add_comma) {
			if (s->trailing_comma) {
				if (s->pos == s->capacity) s->flush_fn(s);
				s->data[s->pos++] = ',';
			}
			jso_indent(s);
		}
	}
}

void jso_array(jso_stream *s)
{
	if (s->capacity - s->pos < 2) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->data[s->pos++] = '[';
	s->add_comma = 0;

	if (s->pretty) jso_prettify_begin_array(s);
}

void jso_object(jso_stream *s)
{
	if (s->capacity - s->pos < 2) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	s->data[s->pos++] = '{';
	s->add_comma = 0;

	if (s->pretty) jso_prettify_begin_object(s);
}

void jso_end_array(jso_stream *s)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	if (s->pretty) jso_prettify_end_array(s);

	s->data[s->pos++] = ']';
	s->add_comma = 1;
}

void jso_end_object(jso_stream *s)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	if (s->pretty) jso_prettify_end_object(s);

	s->data[s->pos++] = '}';
	s->add_comma = 1;
}

void jso_prop(jso_stream *s, const char *key)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	jso_raw_string(s, key);
	s->data[s->pos++] = ':';
	s->add_comma = 0;

	if (s->pretty) {
		if (s->pos == s->capacity) s->flush_fn(s);
		s->data[s->pos++] = ' ';
	}
}

void jso_prop_len(jso_stream *s, const char *key, size_t length)
{
	if (s->pos == s->capacity) s->flush_fn(s);
	if (s->add_comma) s->data[s->pos++] = ',';
	if (s->pretty) jso_prettify(s);
	jso_raw_string_len(s, key, length);
	s->data[s->pos++] = ':';
	s->add_comma = 0;

	if (s->pretty) {
		if (s->pos == s->capacity) s->flush_fn(s);
		s->data[s->pos++] = ' ';
	}
}

void jso_single_line(jso_stream *s)
{
	if (s->single_line_level == 0) {
		s->single_line_level = s->level + 1;
	}
}
