class MaterialsController < ApplicationController
  load_and_authorize_resource :course
  load_and_authorize_resource :material, through: :course
  before_action :add_course_breadcrumbs
  before_action -> { breadcrumb 'Materials', [@course, :materials] }
  before_action -> { breadcrumb @material.name, [@course, @material] }, only: [:show]
  def index; end

  def show; end
end
