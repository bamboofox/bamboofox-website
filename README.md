# bamboofox-website

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

