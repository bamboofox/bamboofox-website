class ApplicationController < ActionController::Base
  protect_from_forgery with: :exception
  add_breadcrumb 'Home', :root_path
  def add_course_breadcrumbs
    breadcrumb 'Courses', courses_path
    breadcrumb @course.name, @course
  end

  def require_login!
    redirect_to new_user_session_path, notice: 'You need to sign in or sign up before continuing.' unless user_signed_in?
  end
end
