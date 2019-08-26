#include <chunkio.h>

VALUE cCIO_Chunk;

void *chunkio_chunk_free(struct cio_chunk *ch)
{
    if (ch) {
        cio_chunk_sync(ch);
        cio_chunk_close(ch, CIO_FALSE);
    }
}

static VALUE chunkio_chunk_close(struct cio_chunk *chunk)
{
    chunkio_chunk_free(chunk);
    return 0;
}

static VALUE chunkio_chunk_allocate_context(VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &chunkio_chunk_type, 0);
}

static VALUE chunkio_chunk_initialize(VALUE self, VALUE context, VALUE stream, VALUE name)
{
    struct cio_ctx *ctx = UnwrapChunkIOContext(context);
    struct cio_stream *st = UnwrapChunkIOStream(stream);
    const char *c_name = RSTRING_PTR(name);

    struct cio_chunk *chunk = cio_chunk_open(ctx, st, c_name, CIO_OPEN, 1000);

    DATA_PTR(self) = chunk;
    return self;
}

static VALUE chunkio_chunk_write(VALUE self, VALUE buf)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);
    ssize_t len = RSTRING_LEN(buf);

    cio_chunk_write(chunk, (void *)RSTRING_PTR(buf), len);
    return INT2NUM(len);
}

static VALUE chunkio_chunk_write_at(VALUE self, VALUE buf, VALUE offset)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);
    ssize_t len = RSTRING_LEN(buf);
    off_t os = NUM2INT(offset);

    int ret = cio_chunk_write_at(chunk, os, (void *)RSTRING_PTR(buf), len);
    return INT2NUM(len);
}

static VALUE chunkio_chunk_size(VALUE self)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);
    size_t size = cio_chunk_get_content_size(chunk);

    return INT2NUM(size);
}

static VALUE chunkio_chunk_set_metadata(VALUE self, VALUE buf)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);
    ssize_t len = RSTRING_LEN(buf);

    int ret = cio_meta_write(chunk, (void *)RSTRING_PTR(buf), len);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to set metadata");
    }
    return INT2NUM(len);
}

static VALUE chunkio_chunk_metadata(VALUE self)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);

    char *buf = NULL;
    size_t size = 0;
    int ret = cio_meta_read(chunk, &buf, &size);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to get metadata");
    }
    return rb_usascii_str_new(buf, size);
}

static VALUE chunkio_chunk_pos(VALUE self)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);
    size_t pos = cio_chunk_get_content_end_pos(chunk);

    return INT2NUM(pos);
}

static VALUE chunkio_chunk_sync(VALUE self)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);
    size_t pos = cio_chunk_sync(chunk);

    return Qnil;
}

static VALUE chunkio_chunk_get_data(VALUE self)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);

    char *buf = NULL;
    size_t size = 0;
    int ret = cio_chunk_get_content(chunk, &buf, &size);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to get data");
    }

    return rb_usascii_str_new(buf, size);
}

static VALUE chunkio_chunk_tx_begin(VALUE self)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);

    int ret = cio_chunk_tx_begin(chunk);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to begin transaction");
    }

    return Qnil;
}

static VALUE chunkio_chunk_tx_commit(VALUE self)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);

    int ret = cio_chunk_tx_commit(chunk);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to commit transaction");
    }
    return Qnil;
}

static VALUE chunkio_chunk_tx_rollback(VALUE self)
{
    struct cio_chunk *chunk;
    TypedData_Get_Struct(self, struct cio_chunk, &chunkio_chunk_type, chunk);

    int ret = cio_chunk_tx_rollback(chunk);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to rollback transaction");
    }
    return Qnil;
}

void Init_chunkio_chunk(VALUE mChunkIO)
{
    cCIO_Chunk = rb_define_class_under(mChunkIO, "Chunk", rb_cObject);
    rb_define_alloc_func(cCIO_Chunk, chunkio_chunk_allocate_context);

    rb_define_method(cCIO_Chunk, "initialize", chunkio_chunk_initialize, 3);
    rb_define_method(cCIO_Chunk, "write", chunkio_chunk_write, 1);
    rb_define_method(cCIO_Chunk, "write_at", chunkio_chunk_write_at, 2);
    rb_define_method(cCIO_Chunk, "size", chunkio_chunk_size, 0);
    rb_define_method(cCIO_Chunk, "set_metadata", chunkio_chunk_set_metadata, 1);
    rb_define_method(cCIO_Chunk, "metadata", chunkio_chunk_metadata, 0);
    rb_define_method(cCIO_Chunk, "sync", chunkio_chunk_sync, 0);
    rb_define_method(cCIO_Chunk, "data", chunkio_chunk_get_data, 0);
    rb_define_method(cCIO_Chunk, "tx_begin", chunkio_chunk_tx_begin, 0);
    rb_define_method(cCIO_Chunk, "tx_commit", chunkio_chunk_tx_commit, 0);
    rb_define_method(cCIO_Chunk, "tx_rollback", chunkio_chunk_tx_rollback, 0);
    rb_define_method(cCIO_Chunk, "close", chunkio_chunk_close, 0);
}
