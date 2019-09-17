# frozen_string_literal: true

require 'chunkio.so'

module ChunkIO
  class ChunkIO
    MAX_CHUNKS = 8196

    def initialize(context_path:, stream_name:, max_chunks: MAX_CHUNKS)
      @ctx = ::ChunkIO::Context.new(context_path)
      @ctx.max_chunks = max_chunks

      @stream = ::ChunkIO::Stream.new(@ctx, stream_name)
    end

    def create_chunk(name:)
      ::ChunkIO::Chunk.new(@ctx, @stream, name)
    end
  end
end
