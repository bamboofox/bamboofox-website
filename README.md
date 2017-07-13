# bamboofox-website

## Requirements

```bash
sudo apt install imagemagick
sudo apt install libmysqlclient-dev
```

## Ruby and Rails versions

Ruby 2.4.0 or newer
Rails 5.1.0 or newer

# Configuration

## Install rvm ( recommened ) and Ruby 2.4.0

```bash
gpg --keyserver hkp://keys.gnupg.net --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3
\curl -sSL https://get.rvm.io | bash -s stable
echo "source ~/.rvm/scripts/rvm" >> .bashrc
rvm install 2.4.0
rvm --default use 2.4.0
```

## Install Rails 5.1.0

```bash
sudo apt-get install -y nodejs
gem install rails --version=5.1.0 --no-rdoc --no-ri
```

## Installation

```bash
bundle install
rake db:migrate
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
