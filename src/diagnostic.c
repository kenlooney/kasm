#include "kasm/diagnostic.h"
#include "kasm/cursor.h"
#include <stdio.h>

void diagnostic(const Source *source, Span span, const char *message) {
    Cursor cursor = cursor_start(source);
    while (cursor.offset < span.start && cursor_peek(&cursor) != '\0')
        cursor_advance(&cursor);
    fprintf(stderr, "%s:%zu:%zu: error: %s [bytes %zu..%zu)\n", source->path, cursor.line,
            cursor.column, message, span.start, span.end);
}
