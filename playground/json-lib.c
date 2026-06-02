/*
 * json_utils.c — implementation of the json_utils library.
 *
 * Build (example):
 *   gcc -Wall -Wextra -o example json_utils.c example.c
 */

#include "json_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── internal helpers ──────────────────────────────────────────────────── */

/* Print a formatted error to stderr (mirrors PHP handleError). */
static void handle_error(const char *msg) {
    fprintf(stderr,
        "\n╔══════════════════════════════════════╗\n"
        "║           JSON Library Error         ║\n"
        "╚══════════════════════════════════════╝\n"
        "  %s\n\n", msg);
}

/* Return 1 if filename ends with ".json" (case-insensitive). */
static int has_json_ext(const char *filename) {
    size_t len = strlen(filename);
    if (len < 5) return 0;
    const char *ext = filename + len - 5;
    return (tolower((unsigned char)ext[0]) == '.' &&
            tolower((unsigned char)ext[1]) == 'j' &&
            tolower((unsigned char)ext[2]) == 's' &&
            tolower((unsigned char)ext[3]) == 'o' &&
            tolower((unsigned char)ext[4]) == 'n');
}

/* Validate filename and existence; return 1 if OK, 0 otherwise. */
static int validate_file(const char *filename) {
    if (!has_json_ext(filename)) {
        handle_error("File extension error. It must be a .json file.");
        return 0;
    }
    FILE *f = fopen(filename, "r");
    if (!f) {
        handle_error("The file does not exist in the specified directory, "
                     "or the filename/extension is incorrect.");
        return 0;
    }
    fclose(f);
    return 1;
}

/* Read entire file into a newly-allocated string (caller must free). */
static char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = (char *)malloc((size_t)(size + 1));
    if (!buf) { fclose(f); return NULL; }

    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

/* Write string to file; return 1 on success. */
static int write_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "wb");
    if (!f) { handle_error("Could not open file for writing."); return 0; }
    fputs(content, f);
    fclose(f);
    return 1;
}

/* ── minimal JSON parser ────────────────────────────────────────────────
 *
 *  Parses:   [ { "key": "value", ... }, ... ]
 *  - skips whitespace
 *  - values are always treated as strings
 *  - no nesting, no arrays-as-values
 */

static const char *skip_ws(const char *p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

/* Parse a quoted JSON string; advance *p past the closing quote.
   Returns heap-allocated unescaped string, or NULL on error. */
static char *parse_string(const char **p) {
    const char *s = *p;
    if (*s != '"') return NULL;
    s++; /* skip opening quote */

    /* measure length */
    const char *start = s;
    size_t len = 0;
    while (*s && *s != '"') {
        if (*s == '\\') { s++; if (!*s) return NULL; }
        len++;
        s++;
    }
    if (*s != '"') return NULL;

    /* copy with basic unescaping */
    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;
    s = start;
    size_t i = 0;
    while (*s && *s != '"') {
        if (*s == '\\') {
            s++;
            switch (*s) {
                case '"':  out[i++] = '"';  break;
                case '\\': out[i++] = '\\'; break;
                case '/':  out[i++] = '/';  break;
                case 'n':  out[i++] = '\n'; break;
                case 'r':  out[i++] = '\r'; break;
                case 't':  out[i++] = '\t'; break;
                default:   out[i++] = *s;  break;
            }
        } else {
            out[i++] = *s;
        }
        s++;
    }
    out[i] = '\0';
    *p = s + 1; /* advance past closing quote */
    return out;
}

/* Parse a JSON value: quoted string or bare token (number/bool/null).
   Returns heap-allocated string. */
static char *parse_value(const char **p) {
    const char *s = skip_ws(*p);
    if (*s == '"') {
        *p = s;
        return parse_string(p);
    }
    /* bare token */
    const char *start = s;
    while (*s && *s != ',' && *s != '}' && *s != ']' && !isspace((unsigned char)*s))
        s++;
    size_t len = (size_t)(s - start);
    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    *p = s;
    return out;
}

/* Parse one JSON object { "k":"v", ... }; advance *p past closing }. */
static int parse_object(const char **p, JsonObject *obj) {
    const char *s = skip_ws(*p);
    if (*s != '{') return 0;
    s++;

    obj->fields      = NULL;
    obj->field_count = 0;

    while (1) {
        s = skip_ws(s);
        if (*s == '}') { s++; break; }
        if (*s == ',') { s++; continue; }

        /* key */
        char *key = parse_string(&s);
        if (!key) return 0;

        s = skip_ws(s);
        if (*s != ':') { free(key); return 0; }
        s++;

        /* value */
        s = skip_ws(s);
        char *val = parse_value(&s);
        if (!val) { free(key); return 0; }

        /* store field */
        obj->fields = (JsonField *)realloc(obj->fields,
                       (obj->field_count + 1) * sizeof(JsonField));
        if (!obj->fields) { free(key); free(val); return 0; }
        obj->fields[obj->field_count].key   = key;
        obj->fields[obj->field_count].value = val;
        obj->field_count++;
    }
    *p = s;
    return 1;
}

/* Parse the top-level array [ {...}, {...}, ... ]. */
static int parse_array(const char *src, JsonArray *arr) {
    arr->objects      = NULL;
    arr->object_count = 0;

    const char *p = skip_ws(src);
    if (*p != '[') {
        handle_error("JSON parse error: expected '[' at start of file.");
        return 0;
    }
    p++;

    while (1) {
        p = skip_ws(p);
        if (*p == ']') break;
        if (*p == ',') { p++; continue; }

        JsonObject obj;
        if (!parse_object(&p, &obj)) {
            handle_error("JSON parse error: malformed object.");
            return 0;
        }
        arr->objects = (JsonObject *)realloc(arr->objects,
                        (arr->object_count + 1) * sizeof(JsonObject));
        if (!arr->objects) return 0;
        arr->objects[arr->object_count++] = obj;
    }
    return 1;
}

/* ── JSON serialiser ────────────────────────────────────────────────────
 *
 *  Writes the JsonArray back to a pretty-printed JSON string.
 *  Returns heap-allocated string; caller must free.
 */

/* Append src to *buf; grow if needed. */
static int buf_append(char **buf, size_t *len, size_t *cap, const char *src) {
    size_t add = strlen(src);
    while (*len + add + 1 > *cap) {
        *cap *= 2;
        char *nb = (char *)realloc(*buf, *cap);
        if (!nb) return 0;
        *buf = nb;
    }
    memcpy(*buf + *len, src, add + 1);
    *len += add;
    return 1;
}

/* Escape a string for JSON output. Returns heap-allocated string. */
static char *escape_string(const char *s) {
    size_t len = strlen(s);
    char *out  = (char *)malloc(len * 2 + 3); /* worst case: every char escaped */
    if (!out) return NULL;
    size_t i = 0;
    out[i++] = '"';
    for (size_t j = 0; j < len; j++) {
        unsigned char c = (unsigned char)s[j];
        switch (c) {
            case '"':  out[i++] = '\\'; out[i++] = '"';  break;
            case '\\': out[i++] = '\\'; out[i++] = '\\'; break;
            case '\n': out[i++] = '\\'; out[i++] = 'n';  break;
            case '\r': out[i++] = '\\'; out[i++] = 'r';  break;
            case '\t': out[i++] = '\\'; out[i++] = 't';  break;
            default:   out[i++] = (char)c;               break;
        }
    }
    out[i++] = '"';
    out[i]   = '\0';
    return out;
}

static char *serialise_array(const JsonArray *arr) {
    size_t cap = 4096, len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) return NULL;
    buf[0] = '\0';

#define APPEND(s) if (!buf_append(&buf, &len, &cap, (s))) { free(buf); return NULL; }

    APPEND("[\n");
    for (size_t i = 0; i < arr->object_count; i++) {
        const JsonObject *obj = &arr->objects[i];
        APPEND("    {\n");
        for (size_t j = 0; j < obj->field_count; j++) {
            char *ek = escape_string(obj->fields[j].key);
            char *ev = escape_string(obj->fields[j].value);
            if (!ek || !ev) { free(ek); free(ev); free(buf); return NULL; }
            APPEND("        ");
            APPEND(ek); APPEND(": "); APPEND(ev);
            free(ek); free(ev);
            if (j + 1 < obj->field_count) APPEND(",");
            APPEND("\n");
        }
        APPEND("    }");
        if (i + 1 < arr->object_count) APPEND(",");
        APPEND("\n");
    }
    APPEND("]\n");
#undef APPEND
    return buf;
}

/* ── copy helpers ───────────────────────────────────────────────────────── */

static char *dup_str(const char *s) {
    size_t n = strlen(s) + 1;
    char *d = (char *)malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

static int copy_object(JsonObject *dst, const JsonObject *src) {
    dst->field_count = src->field_count;
    dst->fields = (JsonField *)malloc(src->field_count * sizeof(JsonField));
    if (!dst->fields) return 0;
    for (size_t i = 0; i < src->field_count; i++) {
        dst->fields[i].key   = dup_str(src->fields[i].key);
        dst->fields[i].value = dup_str(src->fields[i].value);
        if (!dst->fields[i].key || !dst->fields[i].value) return 0;
    }
    return 1;
}

/* ── public API ─────────────────────────────────────────────────────────── */

int json_read(const char *filename, JsonArray *out) {
    if (!validate_file(filename)) return 0;

    char *src = read_file(filename);
    if (!src) { handle_error("Could not read file contents."); return 0; }

    /* empty file → treat as empty array */
    const char *p = src;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') {
        free(src);
        out->objects      = NULL;
        out->object_count = 0;
        return 1;
    }

    int ok = parse_array(src, out);
    free(src);
    return ok;
}

/* Internal: read array from file, call modifier callback, write back. */
typedef int (*modifier_fn)(JsonArray *arr, void *ctx);

static int read_modify_write(const char *filename, modifier_fn fn, void *ctx) {
    if (!validate_file(filename)) return 0;

    char *src = read_file(filename);
    if (!src) { handle_error("Could not read file contents."); return 0; }

    JsonArray arr;
    /* empty file → start with empty array */
    const char *p = src;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') {
        arr.objects      = NULL;
        arr.object_count = 0;
    } else {
        if (!parse_array(src, &arr)) { free(src); return 0; }
    }
    free(src);

    if (!fn(&arr, ctx)) { json_free(&arr); return 0; }

    char *out = serialise_array(&arr);
    json_free(&arr);
    if (!out) { handle_error("Serialisation failed."); return 0; }

    int ok = write_file(filename, out);
    free(out);
    return ok;
}

/* --- json_push --- */
static int do_push(JsonArray *arr, void *ctx) {
    const JsonObject *obj = (const JsonObject *)ctx;
    JsonObject copy;
    if (!copy_object(&copy, obj)) return 0;
    arr->objects = (JsonObject *)realloc(arr->objects,
                   (arr->object_count + 1) * sizeof(JsonObject));
    if (!arr->objects) return 0;
    arr->objects[arr->object_count++] = copy;
    return 1;
}

int json_push(const char *filename, const JsonObject *obj) {
    return read_modify_write(filename, do_push, (void *)obj);
}

/* --- json_unshift --- */
static int do_unshift(JsonArray *arr, void *ctx) {
    const JsonObject *obj = (const JsonObject *)ctx;
    JsonObject copy;
    if (!copy_object(&copy, obj)) return 0;
    arr->objects = (JsonObject *)realloc(arr->objects,
                   (arr->object_count + 1) * sizeof(JsonObject));
    if (!arr->objects) return 0;
    /* shift existing objects right */
    memmove(arr->objects + 1, arr->objects,
            arr->object_count * sizeof(JsonObject));
    arr->objects[0] = copy;
    arr->object_count++;
    return 1;
}

int json_unshift(const char *filename, const JsonObject *obj) {
    return read_modify_write(filename, do_unshift, (void *)obj);
}

/* --- json_delete --- */
typedef struct { const char *key; const char *value; } KV;

static int do_delete(JsonArray *arr, void *ctx) {
    KV *kv = (KV *)ctx;
    for (size_t i = 0; i < arr->object_count; i++) {
        JsonObject *o = &arr->objects[i];
        for (size_t j = 0; j < o->field_count; j++) {
            if (strcmp(o->fields[j].key,   kv->key)   == 0 &&
                strcmp(o->fields[j].value, kv->value) == 0) {
                /* free this object's memory */
                json_free_object(o);
                /* shift remaining objects left */
                memmove(arr->objects + i, arr->objects + i + 1,
                        (arr->object_count - i - 1) * sizeof(JsonObject));
                arr->object_count--;
                return 1; /* delete only first match, like PHP version */
            }
        }
    }
    return 1; /* not found is not an error */
}

int json_delete(const char *filename, const char *key, const char *value) {
    KV kv = { key, value };
    return read_modify_write(filename, do_delete, &kv);
}

/* --- json_update --- */
typedef struct {
    const char *search_key;
    const char *search_value;
    const char *target_key;
    const char *new_value;
} UpdateCtx;

static int do_update(JsonArray *arr, void *ctx) {
    UpdateCtx *u = (UpdateCtx *)ctx;
    for (size_t i = 0; i < arr->object_count; i++) {
        JsonObject *o = &arr->objects[i];
        int found = 0;
        for (size_t j = 0; j < o->field_count; j++) {
            if (strcmp(o->fields[j].key,   u->search_key)   == 0 &&
                strcmp(o->fields[j].value, u->search_value) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) continue;

        /* look for target_key and update in-place */
        for (size_t j = 0; j < o->field_count; j++) {
            if (strcmp(o->fields[j].key, u->target_key) == 0) {
                free(o->fields[j].value);
                o->fields[j].value = dup_str(u->new_value);
                return 1;
            }
        }
        /* target_key not present → add it */
        o->fields = (JsonField *)realloc(o->fields,
                    (o->field_count + 1) * sizeof(JsonField));
        if (!o->fields) return 0;
        o->fields[o->field_count].key   = dup_str(u->target_key);
        o->fields[o->field_count].value = dup_str(u->new_value);
        o->field_count++;
        return 1;
    }
    return 1; /* not found is not an error */
}

int json_update(const char *filename,
                const char *search_key, const char *search_value,
                const char *target_key, const char *new_value) {
    UpdateCtx ctx = { search_key, search_value, target_key, new_value };
    return read_modify_write(filename, do_update, &ctx);
}

/* ── memory management ──────────────────────────────────────────────────── */

void json_free_object(JsonObject *obj) {
    if (!obj) return;
    for (size_t i = 0; i < obj->field_count; i++) {
        free(obj->fields[i].key);
        free(obj->fields[i].value);
    }
    free(obj->fields);
    obj->fields      = NULL;
    obj->field_count = 0;
}

void json_free(JsonArray *arr) {
    if (!arr) return;
    for (size_t i = 0; i < arr->object_count; i++)
        json_free_object(&arr->objects[i]);
    free(arr->objects);
    arr->objects      = NULL;
    arr->object_count = 0;
}

/* ── convenience helpers ────────────────────────────────────────────────── */

JsonObject json_make_object(const char **keys, const char **values) {
    JsonObject obj = { NULL, 0 };
    size_t count = 0;
    while (keys[count]) count++;

    obj.fields = (JsonField *)malloc(count * sizeof(JsonField));
    if (!obj.fields) return obj;
    obj.field_count = count;

    for (size_t i = 0; i < count; i++) {
        obj.fields[i].key   = dup_str(keys[i]);
        obj.fields[i].value = dup_str(values[i]);
    }
    return obj;
}

void json_print(const JsonArray *arr) {
    if (!arr || arr->object_count == 0) {
        printf("(empty array)\n");
        return;
    }
    printf("[\n");
    for (size_t i = 0; i < arr->object_count; i++) {
        const JsonObject *o = &arr->objects[i];
        printf("  {\n");
        for (size_t j = 0; j < o->field_count; j++) {
            printf("    \"%s\": \"%s\"", o->fields[j].key, o->fields[j].value);
            if (j + 1 < o->field_count) printf(",");
            printf("\n");
        }
        printf("  }");
        if (i + 1 < arr->object_count) printf(",");
        printf("\n");
    }
    printf("]\n");
}