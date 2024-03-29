#include <chunkio.h>

VALUE cCIO_Context;

void *chunkio_context_free(struct cio_ctx *ctx)
{
    /*
      Don't call cio_stream_destroy_all(ctx).
      cio_stream is freed by chunkio_stream.
    */

    if (ctx) {
        free(ctx->root_path);
        free(ctx);
        ctx = NULL;
    }
}

static int log_cb(struct cio_ctx *ctx, int level, const char *file, int line, char *str)
{
    (void) ctx;

    printf("[chunkio] %-60s => %s:%i\n",  str, file, line);
    return 0;
}

struct cio_ctx* UnwrapChunkIOContext(VALUE self)
{
    struct cio_ctx *ctx;
    TypedData_Get_Struct(self, struct cio_ctx, &chunkio_context_type, ctx);

    return ctx;
}

static VALUE allocate_context(VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &chunkio_context_type, 0);
}

static VALUE chunkio_context_initialize(VALUE self, VALUE root)
{
    char *p = StringValuePtr(root);
    if (strlen(p) == 0) {
        rb_raise(rb_eStandardError, "Context root path is not allowed empty string");
    }

    /* permission is fixed for now */
    rb_funcall(rb_const_get(rb_cObject, rb_intern("FileUtils")), rb_intern("mkdir_p"), 1, root);

    /* struct cio_ctx *ctx = cio_create(p, log_cb, CIO_DEBUG, 0); /\* flag *\/ */
    struct cio_ctx *ctx = cio_create(p, 0, CIO_DEBUG, 0); /* flag */
    if (!ctx) {
        rb_raise(rb_eStandardError, "failed to create cio_ctx");
    }
    DATA_PTR(self) = ctx;
    return Qnil;
}

static VALUE chunkio_context_root_path(VALUE self)
{
    struct cio_ctx *ctx;
    TypedData_Get_Struct(self, struct cio_ctx, &chunkio_context_type, ctx);

    return rb_str_new2(ctx->root_path);
}

static VALUE chunkio_context_max_chunks_assign(VALUE self, VALUE v)
{
    struct cio_ctx *ctx;
    TypedData_Get_Struct(self, struct cio_ctx, &chunkio_context_type, ctx);

    cio_set_max_chunks_up(ctx, NUM2INT(v));
    return self;
}

void Init_chunkio_context(VALUE mChunkIO)
{
    cCIO_Context = rb_define_class_under(mChunkIO, "Context", rb_cObject);
    rb_define_alloc_func(cCIO_Context, allocate_context);

    rb_define_method(cCIO_Context, "initialize", chunkio_context_initialize, 1);
    rb_define_method(cCIO_Context, "root_path", chunkio_context_root_path, 0);
    rb_define_method(cCIO_Context, "max_chunks=", chunkio_context_max_chunks_assign, 1);
}
