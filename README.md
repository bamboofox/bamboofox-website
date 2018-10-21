# bamboofox-website

[![build status][travis-image]][travis-url]
[![Rawsec's CyberSecurity Inventory](https://inventory.rawsec.ml/img/badges/Rawsec-inventoried-FF5050_flat.svg)](https://inventory.rawsec.ml/ctf_platforms.html#bamboofox)
[![GitHub stars](https://img.shields.io/github/stars/bamboofox/bamboofox-website.svg)](https://github.com/bamboofox/bamboofox-website/stargazers)
[![GitHub license](https://img.shields.io/github/license/bamboofox/bamboofox-website.svg)](https://github.com/bamboofox/bamboofox-website/blob/master/LICENSE)

## Requirements

```bash
sudo apt install imagemagick
sudo apt install libmysqlclient-dev
```

## Ruby and Rails versions

Ruby 2.4.0 or newer
Rails 5.1.0 or newer

## Installation

```bash
bundle install
rake db:migrate
```

## Running

```bash
rails server
```

See on http://localhost:3000

## Deploy

Reference `config/application.yml.example`

Create `config/application.yml`

### Setup seed data and compile assets

```bash
bundle install
RAILS_ENV=production rake db:reset DISABLE_DATABASE_ENVIRONMENT_CHECK=1
RAILS_ENV=production rake assets:precompile
```

## Tests

### Run RuboCop

```bash
rake rubocop
```

### Auto-correct RuboCop offenses
```bash
rake rubocop:auto_correct
```

### Run Rails Best Practice

```bash
rake rails_best_practices
```

### Format erb files

```bash
find . -name '*.erb' | xargs htmlbeautifier
```

[travis-image]: https://travis-ci.org/bamboofox/bamboofox-website.svg?branch=master
[travis-url]: https://travis-ci.org/bamboofox/bamboofox-website
