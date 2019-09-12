# frozen_string_literal: true

require 'chunkio.so'

module ChunkIO
  class ChunkIO
    def initialize(context_path:, stream_name:)
      @ctx = ::ChunkIO::Context.new(context_path)
      @stream = ::ChunkIO::Stream.new(@ctx, stream_name)
      @chunks = []
    end

    def create_chunk(name:)
      c = ::ChunkIO::Chunk.new(@ctx, @stream, name)
      @chunks << c
      c
    end
  end
end
