#ifndef RB_CHUNKIO_CHUNKIO_H
#define RB_CHUNKIO_CHUNKIO_H

#include <ruby.h>

#include <chunkio/chunkio.h>
#include <chunkio/chunkio_compat.h>
#include <crc32/crc32.h>
#include <chunkio/cio_crc32.h>
#include <chunkio/cio_log.h>
#include <chunkio/cio_stream.h>
#include <chunkio/cio_chunk.h>
#include <chunkio/cio_meta.h>
#include <chunkio/cio_scan.h>
#include <chunkio/cio_utils.h>

void *chunkio_context_free(struct cio_ctx *ctx);
void *chunkio_stream_free(struct cio_stream *st);
void *chunkio_chunk_free(struct cio_chunk *ch);

static const rb_data_type_t chunkio_context_type =
    {
     "ChunkIO::Context",
     {0, (void (*)(void *))chunkio_context_free, 0,},
     0, 0,
#ifdef RUBY_TYPED_FREE_IMMEDIATELY
     RUBY_TYPED_FREE_IMMEDIATELY,
#endif
    };

static const rb_data_type_t chunkio_stream_type =
    {
     "ChunkIO::Stream",
     {0, (void (*)(void *))chunkio_stream_free, 0,},
     0, 0,
#ifdef RUBY_TYPED_FREE_IMMEDIATELY
     RUBY_TYPED_FREE_IMMEDIATELY,
#endif
    };

static const rb_data_type_t chunkio_chunk_type =
    {
     "ChunkIO::Chunk",
     {0, (void (*)(void *))chunkio_chunk_free, 0,},                /* TODO free ch->name and ch->head */
     0, 0,
#ifdef RUBY_TYPED_FREE_IMMEDIATELY
     RUBY_TYPED_FREE_IMMEDIATELY,
#endif
    };

struct cio_ctx *UnwrapChunkIOContext(VALUE self);
struct cio_stream* UnwrapChunkIOStream(VALUE self);

#endif
