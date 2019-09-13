# frozen_string_literal: true

require 'chunkio'
require 'fileutils'

RSpec.describe ChunkIO::Stream do
  let(:root_path) do
    File.expand_path('../tmp', __dir__)
  end

  let(:cio_path) do
    File.join(root_path, 'cio_stream')
  end

  after do
    FileUtils.rm_r(root_path) rescue nil
  end

  let(:cio_context) do
    ChunkIO::Context.new(cio_path)
  end

  describe '.initialize' do
    let(:stream_name) do
      'stream_init'
    end

    it { expect(described_class.new(cio_context, stream_name)).to be_a(ChunkIO::Stream) }

    it 'raises an error if string is empty' do
      expect { described_class.new(cio_context, '') }.to raise_error(StandardError, /is not allowed/)
    end

    it 'raises an error if not a string' do
      expect { described_class.new(cio_context, nil) }.to raise_error(TypeError)
    end
  end
end
