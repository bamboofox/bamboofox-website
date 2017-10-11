class CoursesController < ApplicationController
  load_and_authorize_resource
  breadcrumb 'Courses', :courses_path
  def index; end

  def show
    breadcrumb @course.name, @course
  end
end
