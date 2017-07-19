require 'redcarpet'
require 'rouge'
require 'rouge/plugins/redcarpet'

class Html < Redcarpet::Render::HTML
  include Rouge::Plugins::Redcarpet
end
