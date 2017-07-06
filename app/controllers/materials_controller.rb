class MaterialsController < ApplicationController
  load_and_authorize_resource :course
  load_and_authorize_resource :material, through: :course

  def index; end

  def show; end
end
