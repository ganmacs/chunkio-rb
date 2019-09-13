# frozen_string_literal: true

require 'chunkio'
require 'fileutils'
require 'json'

RSpec.describe ChunkIO::Chunk do
  # https://github.com/edsiper/chunkio/tree/1452f4175d808a855c80d3f93ae83383a4b8cd90#file-layout
  METADATA_INDEX = 2 + 4 + 16 + 2 # header(2)+crc32(4)+padding(16)+metadata_len(2)

  let(:root_path) do
    File.expand_path('../tmp', __dir__)
  end

  let(:cio_path) do
    File.join(root_path, 'cio_chunk')
  end

  after do
    FileUtils.rm_r(root_path) rescue nil
  end

  let(:stream_name) do
    'stream'
  end

  let(:chunk_name) do
    'chunk_name'
  end

  let(:full_path) do
    File.join(cio_path, stream_name, chunk_name)
  end

  let(:cio_context) do
    ChunkIO::Context.new(cio_path)
  end

  let(:cio_stream) do
    ChunkIO::Stream.new(cio_context, stream_name)
  end

  describe '.initialize' do
    let(:chunk_name) do
      'init'
    end

    it { expect(described_class.new(cio_context, cio_stream, chunk_name)).to be_a(ChunkIO::Chunk) }

    it 'create chunk file' do
      expect { described_class.new(cio_context, cio_stream, chunk_name) }.to change { File.exist?(full_path) }.from(false).to(true)
    end

    it 'raises an error if string is empty' do
      expect { described_class.new(cio_context, cio_stream, '') }.to raise_error(StandardError, /is not allowed/)
    end

    it 'raises an error if string is empty' do
      expect { described_class.new(cio_context, cio_stream, nil) }.to raise_error(TypeError)
    end

    context 'when existing file exists' do
      let(:data_body) do
        'this is data'
      end

      let(:meta_body) do
        'this is meatdata'
      end

      before do
        c = described_class.new(cio_context, cio_stream, chunk_name)
        c.write(data_body)
        c.set_metadata(meta_body)
        c.close
      end

      it 'load all data' do
        c = described_class.new(cio_context, cio_stream, chunk_name)
        expect(c.data).to eq(data_body)
        expect(c.metadata).to eq(meta_body)
      ensure
        c.unlink
      end

      context 'when invalid data format' do
        it 'raises an error' do
          File.open(full_path, 'w') do |f|
            f.write ['test', Time.now.to_i, { 'message' => 'hi' }].to_json
          end

          expect { described_class.new(cio_context, cio_stream, chunk_name) }.to raise_error(StandardError, /Failed to create/)
        end
      end
    end
  end

  describe '#write' do
    let(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    after do
      chunk.unlink rescue nil
    end

    it 'writes data to file' do
      expect(chunk.write('test1')).to eq(5)
      chunk.sync

      File.open(File.join(cio_path, stream_name, chunk_name), 'r') do |f|
        expect(f.read[-5..-1]).to eq('test1')
      end
    end

    it 'raise an error when given not string ' do
      expect { chunk.write(nil) }.to raise_error(TypeError)
    end
  end

  describe '#unlink' do
    let(:chunk_name) do
      'unlink'
    end

    after do
      chunk.unlink rescue nil
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    it { expect { chunk.unlink }.to change { File.exist?(full_path) }.from(true).to(false) }

    it 'raise an error if called twice' do
      chunk.unlink

      expect { chunk.unlink }.to raise_error(IOError, /already closed/)
    end
  end

  describe '#close' do
    let(:chunk_name) do
      'close'
    end

    after do
      chunk.unlink rescue nil
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    it 'closes IO' do
      chunk.close
      expect { chunk.write('test') }.to raise_error(IOError, /already closed/)
    end

    it 'ignores sencond calling' do
      expect(chunk.close).to eq(nil)
      expect(chunk.close).to eq(nil)
    end
  end

  describe '#bytesize' do
    let(:chunk_name) do
      'bytesize'
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    after do
      chunk.unlink rescue nil
    end

    it 'returns bytes size' do
      expect(chunk.bytesize).to eq(0)
      s1 = 'apple'
      s2 = 'apple2'
      s3 = 'アップル'
      chunk.write(s1)
      expect(chunk.bytesize).to eq(s1.bytesize)

      chunk.write(s2)
      expect(chunk.bytesize).to eq(s1.bytesize + s2.bytesize)

      chunk.write(s3)
      expect(chunk.bytesize).to eq(s1.bytesize + s2.bytesize + s3.bytesize)
    end
  end

  describe '#set_metadata' do
    let(:chunk_name) do
      'set_metadata'
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    after do
      chunk.unlink rescue nil
    end

    it 'writes metadata to file' do
      chunk.set_metadata('a' * 10)
      chunk.sync
      File.open(File.join(cio_path, stream_name, chunk_name), 'r') do |f|
        expect(f.read.slice(METADATA_INDEX, 10)).to eq(('a' * 10).b)
      end
    end

    it 'overwrite all metadata to file' do
      chunk.set_metadata('a' * 10)
      chunk.sync
      chunk.set_metadata('b' * 5)
      chunk.sync
      File.open(File.join(cio_path, stream_name, chunk_name), 'r') do |f|
        # should be 10
        expect(f.read.slice(METADATA_INDEX, 10)).to eq(('b' * 5).b)
      end
    end

    it 'raise an error when given not string ' do
      expect { chunk.set_metadata(nil) }.to raise_error(TypeError)
    end
  end

  describe '#metadata' do
    let(:chunk_name) do
      'metadata'
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    after do
      chunk.unlink rescue nil
    end

    it 'read written metadata' do
      s = 'a' * 10
      chunk.set_metadata(s)
      expect(chunk.metadata).to eq(s)
    end

    context 'when metadata is empty' do
      it { expect(chunk.metadata).to eq('') }
    end

    context 'when loading exsiting data' do
      let(:metadata_body) do
        'this is metadata body'
      end

      before do
        c = described_class.new(cio_context, cio_stream, chunk_name)
        c.set_metadata(metadata_body)
        c.close
      end

      it 'loads written metadata' do
        expect(chunk.metadata).to eq(metadata_body)
      end

      context 'when metadata is empty' do
        let(:metadata_body) do
          ''
        end

        it { expect(chunk.metadata).to eq('') }
      end
    end
  end

  describe '#data' do
    let(:chunk_name) do
      'data'
    end

    after do
      chunk.unlink rescue nil
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    it 'reads written data' do
      s = 'a' * 10
      chunk.write(s)
      expect(chunk.data).to eq(s)
    end

    it 'can read multiple time' do
      s = 'a' * 10
      chunk.write(s)
      expect(chunk.data).to eq(s)
      expect(chunk.data).to eq(s)
    end

    context 'when data is empty' do
      it { expect(chunk.data).to eq('') }
    end

    context 'when loading exsiting data' do
      let(:data_body) do
        'this is data body'
      end

      before do
        chunk.write(data_body)
        chunk.sync
      end

      it 'loads written data' do
        c = described_class.new(cio_context, cio_stream, chunk_name)
        expect(c.data).to eq(data_body)
        # exit
      end

      context 'when data is empty' do
        let(:data_body) do
          ''
        end

        it { expect(chunk.data).to eq('') }
      end
    end
  end

  describe '#tx_begin' do
    let(:chunk_name) do
      'tx_begin'
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    after do
      chunk.unlink rescue nil
    end

    it 'does not have reenterant' do
      chunk.tx_begin
      expect { chunk.tx_begin }.to raise_error(StandardError, /Failed to begin/)
    end

    it 'raise an error if chunk is closed' do
      chunk.close
      expect { chunk.tx_begin }.to raise_error(StandardError, /already closed/)
    end
  end

  describe '#tx_rollback' do
    let(:chunk_name) do
      'tx_rollback'
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    after do
      chunk.unlink rescue nil
    end

    it 'can rollback writing data' do
      chunk.tx_begin

      data = 'message'
      chunk.write(data)
      expect(chunk.data).to eq(data)
      expect(chunk.bytesize).to eq(data.bytesize)

      chunk.tx_rollback
      expect(chunk.data).to eq('')
      expect(chunk.bytesize).to eq(0)
    end

    it "can't rollback after commit" do
      chunk.tx_begin

      data = 'message'
      chunk.write(data)
      expect(chunk.data).to eq(data)
      expect(chunk.bytesize).to eq(data.bytesize)

      chunk.tx_commit
      expect(chunk.data).to eq(data)
      expect(chunk.bytesize).to eq(data.bytesize)

      chunk.tx_begin
      chunk.write(data)
      expect(chunk.data).to eq(data * 2)
      expect(chunk.bytesize).to eq(data.bytesize * 2)

      chunk.tx_rollback
      expect(chunk.data).to eq(data)
      expect(chunk.bytesize).to eq(data.bytesize)
    end

    context 'when chunk is not transaction' do
      it { expect { chunk.tx_rollback }.to raise_error(StandardError, /Failed to rollback/) }
    end

    context 'when chunk is closed' do
      it 'raise an error' do
        chunk.tx_begin
        chunk.close
        expect { chunk.tx_rollback }.to raise_error(StandardError, /already closed/)
      end
    end
  end

  describe '#sync_mode=' do
    let(:chunk_name) do
      'sync_mode='
    end

    let!(:chunk) do
      described_class.new(cio_context, cio_stream, chunk_name)
    end

    after do
      chunk.unlink rescue nil
    end

    it 'always sync after write' do
      chunk.sync_mode = true
      expect(chunk.write('test1')).to eq(5)

      File.open(File.join(cio_path, stream_name, chunk_name), 'r') do |f|
        expect(f.read[-5..-1]).to eq('test1')
      end
    end

    it 'always sync after set_meta' do
      chunk.sync_mode = true
      chunk.set_metadata('a' * 10)
      chunk.set_metadata('b' * 9)

      File.open(File.join(cio_path, stream_name, chunk_name), 'r') do |f|
        expect(f.read.slice(METADATA_INDEX, 10)).to eq(('b' * 9).b)
      end
    end

    context 'when passed not boolean value' do
      it { expect { chunk.sync_mode = nil }.to raise_error(StandardError, /true or false/) }
    end
  end
end
