class AboutsController < ApplicationController
  def show
    @about = About.first_or_create(content: '')
  end
end
