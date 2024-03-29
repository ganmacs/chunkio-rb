# frozen_string_literal: true

lib = File.expand_path('lib', __dir__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'chunkio/version'

Gem::Specification.new do |spec|
  spec.name          = 'chunkio'
  spec.version       = ChunkIO::VERSION
  spec.authors       = ['Yuta Iwama']
  spec.email         = ['ganmacs@gmail.com']

  spec.summary       = 'Wrapper of chunkio'
  spec.description   = 'Wrapper of chunkio'
  spec.homepage      = 'https://github.com/ganmacs/chunkio-rb'
  spec.license       = 'Apache-2.0'

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files         = Dir.chdir(File.expand_path(__dir__)) do
    `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|spec|features)/}) }
  end
  spec.bindir        = 'exe'
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ['lib']
  spec.extensions = ['ext/chunkio/extconf.rb']

  spec.add_runtime_dependency 'mini_portile2', '>= 2.2.0'

  spec.add_development_dependency 'bundler', '~> 2.0'
  spec.add_development_dependency 'rake', '~> 10.0'
  spec.add_development_dependency 'rake-compiler'
  spec.add_development_dependency 'rspec', '~> 3.0'
  spec.add_development_dependency 'rubocop'
end
