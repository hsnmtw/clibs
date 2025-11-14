#include <stdlib.h>

#define MEMSTREAM_API

// -- INTERFACE
typedef struct {
    char   *buffer;   // Pointer to the memory buffer
    size_t  size;     // Total size of the buffer
    size_t  len;      // length of the buffer
    size_t  position; // Current position in the buffer
} memstream_t;

MEMSTREAM_API memstream_t *memstream_new(size_t size);
MEMSTREAM_API void         memstream_free(memstream_t *stream);
MEMSTREAM_API size_t       memstream_write(memstream_t *stream, const void *ptr, size_t size);
MEMSTREAM_API size_t       memstream_read(memstream_t *stream, void *ptr, size_t size);
MEMSTREAM_API void         memstream_reset(memstream_t *stream);



// -- IMPLEMENTATION



#ifdef  _MEMSTREAM_H_IMPLEMENTATION

// Create a memory stream
MEMSTREAM_API memstream_t *memstream_new(size_t size) {
    memstream_t *stream = (memstream_t *)malloc(sizeof(memstream_t));
    if (!stream) return NULL;

    stream->buffer = (char *)malloc(size);
    memset(stream->buffer,0,size);
    if (!stream->buffer) {
        free(stream);
        return NULL;
    }

    stream->size = size;
    stream->position = 0;
    stream->len = 0;
    return stream;
}

// Free the memory stream
MEMSTREAM_API void memstream_free(memstream_t *stream) {
    if (stream) {
        free(stream->buffer);
        free(stream);
    }
}

// Write to the memory stream
MEMSTREAM_API size_t memstream_write(memstream_t *stream, const void *ptr, size_t size) {
    if (stream->position + size > stream->size) {
        size = stream->size - stream->position; // Limit to buffer size
    }
    
    memcpy(stream->buffer + stream->position, ptr, size);
    stream->position += size;
    stream->len += size;
    return size;
}

// Read from the memory stream
MEMSTREAM_API size_t memstream_read(memstream_t *stream, void *ptr, size_t size) {

    if (stream->position + size > stream->len) {
        size = stream->len - stream->position; // Limit to buffer len
    }
    
    else if (stream->position + size > stream->size) {
        size = stream->size - stream->position; // Limit to buffer size
    }

    //printf("len=%lld, size=%lld, pos=%lld, ret=%lld\n", stream->len, stream->size, stream->position, size);

    memcpy(ptr, stream->buffer + stream->position, size);
    stream->position += size;
    return size;
}

// Reset the position of the memory stream
MEMSTREAM_API void memstream_reset(memstream_t *stream) {
    stream->position = 0;
}

#endif// _MEMSTREAM_H_IMPLEMENTATION
