# frozen_string_literal: true

require 'bundler/gem_tasks'
require 'rake/extensiontask'
require 'rspec/core/rake_task'

task default: :compile

Rake::ExtensionTask.new('chunkio') do |t|
  t.name = 'chunkio'

  t.ext_dir = 'ext/chunkio'
  t.lib_dir = 'lib'
end
