#include <chunkio.h>

VALUE mChunkIO;

void Init_chunkio(void)
{
    mChunkIO = rb_define_module("ChunkIO");
    Init_chunkio_context(mChunkIO);
    Init_chunkio_stream(mChunkIO);
    Init_chunkio_chunk(mChunkIO);
}
