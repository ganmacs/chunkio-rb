# ChunkI/O

This gem is wrapper around [ChunkI/O](https://github.com/edsiper/chunkio).

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'chunkio'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install chunkio

## Usage

```rb
require 'chunkio'
c = ChunkIO.new
c.write("test")
c.set_metadata("this is metadata")
c.close
```

TODO: Write usage instructions here

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake spec` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/ganmacs/chunkio.
