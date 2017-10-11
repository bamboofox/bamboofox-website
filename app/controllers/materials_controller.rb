class MaterialsController < ApplicationController
  load_and_authorize_resource :course
  load_and_authorize_resource :material, through: :course
  before_action :add_course_breadcrumbs

  def index
    breadcrumb 'Materials', [@course, :materials]
  end

  def show
    breadcrumb 'Materials', [@course, :materials]
    breadcrumb @material.name, [@course, @material]
  end
end
