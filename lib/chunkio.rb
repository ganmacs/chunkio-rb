# frozen_string_literal: true

require 'chunkio.so'
require 'chunkio/version'
require 'chunkio/chunkio'

module ChunkIO
  def self.new(context_path:, stream_name:)
    ::ChunkIO::ChunkIO.new(
      context_path: context_path,
      stream_name: stream_name,
    )
  end
end
