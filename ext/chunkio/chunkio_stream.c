#include <chunkio.h>

VALUE cCIO_Stream;

void *chunkio_stream_free(struct cio_stream *st)
{
    /*
      Don't call cio_chunk_close_stream(st).
      cio_chunk is freed by chunkio_chunk.
     */
    if (st) {
        /* destroy stream */
        free(st->name);
        free(st);
        st = NULL;
    }
}

struct cio_stream* UnwrapChunkIOStream(VALUE self)
{
    struct cio_ctx *ctx;
    TypedData_Get_Struct(self, struct cio_stream, &chunkio_stream_type, ctx);

    return ctx;
}

static VALUE allocate_stream(VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &chunkio_stream_type, 0);
}

static VALUE chunkio_stream_initialize(VALUE self, VALUE context, VALUE name)
{
    char *stream_name = StringValuePtr(name);
    if (strlen(stream_name) == 0) {
        rb_raise(rb_eStandardError, "stream_name is not allowed empty string");
    }

    struct cio_ctx *ctx = UnwrapChunkIOContext(context);
    struct cio_stream *st = cio_stream_create(ctx, stream_name, CIO_STORE_FS); /* TODO CIO_STORE_FS */
    if (!st) {
        rb_raise(rb_eStandardError, "chunkio: cannot create stream");
        return Qnil;
    }

    DATA_PTR(self) = st;
    return Qnil;
}


void Init_chunkio_stream(VALUE mChunkIO)
{
    cCIO_Stream = rb_define_class_under(mChunkIO, "Stream", rb_cObject);
    rb_define_alloc_func(cCIO_Stream, allocate_stream);

    rb_define_method(cCIO_Stream, "initialize", chunkio_stream_initialize, 2);
    /* rb_define_method(cCIO_Stream, "root_path", chunkio_stream_root_path, 0); */
}
