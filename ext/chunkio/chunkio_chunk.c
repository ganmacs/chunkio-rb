#include <chunkio.h>

VALUE cCIO_Chunk;

void *chunkio_chunk_free(chunkio_chunk *ch)
{
    if (ch->inner != NULL) {
        /*
          `cio_stream` and `cio_ctx` can be freed before this line.
          but cio_chunk_close and cio_chunk_sync needs them during freeing itself.
          So creating dummy object to work correctly.
        */
        struct cio_stream st;
        st.type = CIO_STORE_FS;
        st.name = (char *)"stream";
        ch->inner->st = &st;

        struct cio_ctx ct;
        ct.flags = CIO_CHECKSUM;
        ct.log_cb = NULL;
        ch->inner->ctx = &ct;

        cio_chunk_sync(ch->inner);
        cio_chunk_close(ch->inner, CIO_FALSE);
        ch->inner = NULL;
    }

    if (ch != NULL) {
        xfree(ch);
        ch = NULL;
    }
}

static VALUE chunkio_chunk_allocate_context(VALUE klass)
{
    chunkio_chunk *c = (chunkio_chunk *)xmalloc(sizeof(struct chunkio_chunk_type));
    c->inner = NULL;
    c->closed = 0;
    c->sync_mode = 0;
    return TypedData_Wrap_Struct(klass, &chunkio_chunk_type, c);
}

static VALUE chunkio_chunk_initialize(VALUE self, VALUE context, VALUE stream, VALUE name)
{
    struct cio_ctx *ctx = UnwrapChunkIOContext(context);
    struct cio_stream *st = UnwrapChunkIOStream(stream);
    const char *c_name = StringValuePtr(name);
    if (strlen(c_name) == 0) {
        rb_raise(rb_eStandardError, "chunk name is not allowed empty string");
    }

    struct cio_chunk *chunk = cio_chunk_open(ctx, st, c_name, CIO_OPEN, 1000);
    if (chunk == NULL) {
        rb_raise(rb_eStandardError, "Failed to create chunk");
    }

    ((chunkio_chunk*)DATA_PTR(self))->inner = chunk;
    return self;
}

static VALUE chunkio_chunk_unlink(VALUE self)
{
    chunkio_chunk *chunk = NULL;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }

    if (cio_chunk_unlink(chunk->inner) == -1) {
        rb_raise(rb_eIOError, "Failed to unlink");
    }

    chunk->closed = 1;          /* mark as closed */
    return Qnil;
}

static VALUE chunkio_chunk_close(VALUE self)
{
    chunkio_chunk *chunk = NULL;
    int type;
    struct cio_file *cf;

    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (!chunk->closed) {
        cio_chunk_sync(chunk->inner);
        type = chunk->inner->st->type;
        if (type == CIO_STORE_FS) {
            /* COPY form chunkio cio_file.c */
            chunk->inner->st->type;
            cf = (struct cio_file *)chunk->inner->backend;
            close(cf->fd);
            cf->fd = 0;
        }
        chunk->closed = 1;      /* mark as close */

        return Qnil;
    }

    return Qnil;
}

static VALUE chunkio_chunk_write(VALUE self, VALUE buf)
{
    chunkio_chunk *chunk = NULL;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }
    Check_Type(buf, T_STRING);

    ssize_t len = RSTRING_LEN(buf);
    cio_chunk_write(chunk->inner, (void *)RSTRING_PTR(buf), len);

    if (chunk->sync_mode) {
        int ret = cio_chunk_sync(chunk->inner);
        if (ret == -1) {
            rb_raise(rb_eStandardError, "failed to sync");
        }
    }
    return INT2NUM(len);
}

/* static VALUE chunkio_chunk_write_at(VALUE self, VALUE buf, VALUE offset) */
/* { */
/*     chunkio_chunk *chunk = NULL; */
/*     TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk); */
/*     ssize_t len = RSTRING_LEN(buf); */
/*     off_t os = NUM2INT(offset); */

/*     int ret = cio_chunk_write_at(chunk, os, (void *)RSTRING_PTR(buf), len); */
/*     return INT2NUM(len); */
/* } */

static VALUE chunkio_chunk_bytesize(VALUE self)
{
    chunkio_chunk *chunk = NULL;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    size_t size = cio_chunk_get_content_size(chunk->inner);

    return INT2NUM(size);
}

static VALUE chunkio_chunk_set_metadata(VALUE self, VALUE buf)
{
    chunkio_chunk *chunk = NULL;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }

    Check_Type(buf, T_STRING);
    ssize_t len = RSTRING_LEN(buf);
    int ret = cio_meta_write(chunk->inner, (void *)RSTRING_PTR(buf), len);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to set metadata");
    }

    if (chunk->sync_mode) {
        int ret = cio_chunk_sync(chunk->inner);
        if (ret == -1) {
            rb_raise(rb_eStandardError, "failed to sync");
        }
    }
    return INT2NUM(len);
}

static VALUE chunkio_chunk_metadata(VALUE self)
{
    chunkio_chunk *chunk = NULL;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }

    char *buf = NULL;
    size_t size = 0;

    /* cio_meta_chunk return -1 if size is 0... */
    if (cio_meta_size(chunk->inner) == 0) {
        return rb_str_new(0, 0);
    }

    int ret = cio_meta_read(chunk->inner, &buf, &size);

    return rb_str_new(buf, size);
}

/* static VALUE chunkio_chunk_pos(VALUE self) */
/* { */
/*     chunkio_chunk *chunk; */
/*     TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk); */
/*     size_t pos = cio_chunk_get_content_end_pos(chunk); */

/*     return INT2NUM(pos); */
/* } */

static VALUE chunkio_chunk_sync(VALUE self)
{
    chunkio_chunk *chunk;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }

    size_t ret = cio_chunk_sync(chunk->inner);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to sync");
    }
    return Qnil;
}

static VALUE chunkio_chunk_get_data(VALUE self)
{
    chunkio_chunk *chunk;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }

    char *buf = NULL;
    size_t size = 0;
    int ret = cio_chunk_get_content(chunk->inner, &buf, &size);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to get data");
    }

    return rb_str_new(buf, size);
}

static VALUE chunkio_chunk_tx_begin(VALUE self)
{
    chunkio_chunk *chunk;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }

    int ret = cio_chunk_tx_begin(chunk->inner);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "Failed to begin transaction");
    }

    return Qnil;
}

static VALUE chunkio_chunk_tx_commit(VALUE self)
{
    chunkio_chunk *chunk;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }

    int ret = cio_chunk_tx_commit(chunk->inner);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "failed to commit transaction");
    }

    if (chunk->sync_mode) {
        int ret = cio_chunk_sync(chunk->inner);
        if (ret == -1) {
            rb_raise(rb_eStandardError, "failed to sync");
        }
    }
    return Qnil;
}

static VALUE chunkio_chunk_tx_rollback(VALUE self)
{
    chunkio_chunk *chunk;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    if (chunk->closed) {
        rb_raise(rb_eIOError, "IO was already closed");
    }

    int ret = cio_chunk_tx_rollback(chunk->inner);
    if (ret == -1) {
        rb_raise(rb_eStandardError, "Failed to rollback transaction");
    }
    return Qnil;
}

static VALUE chunkio_chunk_sync_mode(VALUE self) {
    chunkio_chunk *chunk;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);
    return chunk->sync_mode == 1 ? Qtrue : Qfalse;
}

static VALUE chunkio_chunk_sync_mode_assign(VALUE self, VALUE bool) {
    chunkio_chunk *chunk;
    TypedData_Get_Struct(self, chunkio_chunk, &chunkio_chunk_type, chunk);

    if (bool == Qtrue) {
        chunk->sync_mode = 1;
    } else if (bool == Qfalse){
        chunk->sync_mode = 0;
    } else {
        rb_raise(rb_eTypeError, "expected true or false");
    }

    return Qnil;
}

void Init_chunkio_chunk(VALUE mChunkIO)
{
    cCIO_Chunk = rb_define_class_under(mChunkIO, "Chunk", rb_cObject);
    rb_define_alloc_func(cCIO_Chunk, chunkio_chunk_allocate_context);

    rb_define_method(cCIO_Chunk, "initialize", chunkio_chunk_initialize, 3);
    rb_define_method(cCIO_Chunk, "write", chunkio_chunk_write, 1);
    rb_define_method(cCIO_Chunk, "unlink", chunkio_chunk_unlink, 0);
    rb_define_method(cCIO_Chunk, "close", chunkio_chunk_close, 0);
    rb_define_method(cCIO_Chunk, "bytesize", chunkio_chunk_bytesize, 0);
    rb_define_method(cCIO_Chunk, "set_metadata", chunkio_chunk_set_metadata, 1);
    rb_define_method(cCIO_Chunk, "metadata", chunkio_chunk_metadata, 0);
    rb_define_method(cCIO_Chunk, "sync", chunkio_chunk_sync, 0);
    rb_define_method(cCIO_Chunk, "data", chunkio_chunk_get_data, 0);
    rb_define_method(cCIO_Chunk, "tx_begin", chunkio_chunk_tx_begin, 0);
    rb_define_method(cCIO_Chunk, "tx_commit", chunkio_chunk_tx_commit, 0);
    rb_define_method(cCIO_Chunk, "tx_rollback", chunkio_chunk_tx_rollback, 0);
    rb_define_method(cCIO_Chunk, "sync_mode", chunkio_chunk_sync_mode, 0);
    rb_define_method(cCIO_Chunk, "sync_mode=", chunkio_chunk_sync_mode_assign, 1);
    /* rb_define_method(cCIO_Chunk, "write_at", chunkio_chunk_write_at, 2); */
}
