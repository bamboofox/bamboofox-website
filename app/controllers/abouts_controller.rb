class AboutsController < ApplicationController
  def show
    breadcrumb 'About', about_path
    @about = About.first_or_create(content: '')
  end
end
