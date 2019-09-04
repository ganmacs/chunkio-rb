# frozen_string_literal: true

require 'mkmf'
require 'rubygems'
require 'mini_portile2'

dir_config('chunkio')
message "Building chunkio\n"
recipe = MiniPortileCMake.new('chunkio', 'v0.0.1')

recipe.files << {
  url: 'https://github.com/ganmacs/chunkio/tarball/4773b4e7d27c8b8396f797c09dd22db5ffadeda8',
}

recipe.cook
$LIBPATH = ["#{recipe.path}/lib"] + $LIBPATH
$CPPFLAGS << " -I#{recipe.path}/include"

recipe.activate

unless have_header('chunkio/chunkio.h')
  abort 'chunkio/chunkio.h is not found'
end

unless have_library('chunkio-static')
  abort 'libchunkio-static not found'
end

unless have_library('cio-crc32')
  abort 'libcio-crc32 not found'
end

create_makefile 'chunkio'
