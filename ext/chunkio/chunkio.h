#ifndef RB_CHUNKIO_CHUNKIO_H
#define RB_CHUNKIO_CHUNKIO_H

#include <ruby.h>
#include <chunkio/chunkio.h>
#include <chunkio/cio_meta.h>

typedef struct chunkio_chunk_type {
    struct cio_chunk* inner;
    int closed;
    int sync_mode;              /* 0 is false, 1 is true */
} chunkio_chunk;


void *chunkio_context_free(struct cio_ctx *ctx);
void *chunkio_stream_free(struct cio_stream *st);
void *chunkio_chunk_free(chunkio_chunk *ch);

static const rb_data_type_t chunkio_context_type =
    {
     "ChunkIO::Context",
     {0, (void (*)(void *))chunkio_context_free, 0,},
     0, 0, 0
    };

static const rb_data_type_t chunkio_stream_type =
    {
     "ChunkIO::Stream",
     {0, (void (*)(void *))chunkio_stream_free, 0,},
     0, 0, 0
    };

static const rb_data_type_t chunkio_chunk_type =
    {
     "ChunkIO::Chunk",
     {0, (void (*)(void *))chunkio_chunk_free, 0,},                /* TODO free ch->name and ch->head */
     0, 0, 0
    };

struct cio_ctx *UnwrapChunkIOContext(VALUE self);
struct cio_stream* UnwrapChunkIOStream(VALUE self);

#endif
