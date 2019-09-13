# frozen_string_literal: true

require 'chunkio'
require 'fileutils'

RSpec.describe ChunkIO::Context do
  let(:root_path) do
    File.expand_path('../tmp', __dir__)
  end

  let(:cio_path) do
    File.join(root_path, 'cio_context')
  end

  after do
    FileUtils.rm_r(root_path) rescue nil
  end

  describe '.initialize' do
    it 'creates given directory' do
      expect { described_class.new(cio_path) }.to change { Dir.exist?(cio_path) }.from(false).to(true)
    end

    it 'raises an error if string is empty' do
      expect { described_class.new('') }.to raise_error(StandardError, /is not allowed/)
    end

    it 'raises an error if not a string' do
      expect { described_class.new(nil) }.to raise_error(TypeError)
    end
  end

  describe '#path' do
    it 'returns path' do
      cio = described_class.new(cio_path)
      expect(cio.root_path).to eq(cio_path)
    end
  end
end
