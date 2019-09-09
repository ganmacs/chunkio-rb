# frozen_string_literal: true

require 'chunkio.so'

module ChunkIO
  class ChunkIO
    def initialize(context_path:, stream_name:)
      unless context_path
        raise 'invalid'
      end

      unless stream_name
        raise 'stream'
      end

      if  context_path.empty?
        raise 'context_path should be at least one char'
      end

      if stream_name.empty?
        raise 'stream_name should be at least one char'
      end

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
