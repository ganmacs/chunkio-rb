# frozen_string_literal: true

require 'mkmf'
require 'rubygems'
require 'mini_portile2'

dir_config('chunkio')
message "Building chunkio\n"
recipe = MiniPortileCMake.new('chunkio', 'v0.0.1')

recipe.files << {
  url: 'https://github.com/ganmacs/chunkio/tarball/c0449b5bb59b8c7f56d49b61ebc9faf3e888cdbd',
}

class << recipe
  def compile
    execute('compile', make_cmd)

    execute('extract libchunk-static', 'ar xv src/libchunkio-static.a')
    execute('extract libcio_crc32', 'ar xv deps/crc32/libcio-crc32.a')
    # TODO
    # it can't find the symbol: update_crc without it. so it creates static library by itself
    execute('archive', 'ar vrcs libchunkio.a chunkio.c.o cio_file.c.o cio_memfs.c.o cio_os.c.o cio_stats.c.o cio_utils.c.o cio_chunk.c.o cio_log.c.o cio_meta.c.o cio_scan.c.o cio_stream.c.o crc32.c.o')
  end

  def install
    lib_path = File.join(port_path, 'lib')
    include_path = File.join(port_path, 'include')

    FileUtils.mkdir_p([lib_path, include_path])
    FileUtils.cp(File.join(work_path, 'libchunkio.a'), lib_path)

    FileUtils.cp_r(File.join(work_path, 'include/chunkio'), include_path)
    FileUtils.cp_r(File.join(work_path, 'deps/monkey/include/monkey'), include_path)

    crc32 = File.join(include_path, 'crc32')
    FileUtils.mkdir_p(crc32)
    FileUtils.cp(File.join(work_path, 'deps/crc32/crc32.h'), crc32)
  end
end

recipe.cook
$LIBPATH = ["#{recipe.path}/lib"] + $LIBPATH
$CPPFLAGS << " -I#{recipe.path}/include"

recipe.activate

unless have_header('chunkio/chunkio.h')
  abort 'chunkio/chunkio.h is not found'
end

unless have_library('chunkio')
  abort 'libchunkio-static not found'
end

create_makefile 'chunkio'
