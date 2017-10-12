class AdminController < ApplicationController
  before_action :authenticate_user!
  before_action :require_admin
  layout 'admin'
  add_breadcrumb 'Admin', :admin_root_path
  def add_admin_course_breadcrumbs
    breadcrumb 'Courses', %i[admin courses]
    breadcrumb @course.name, [:admin, @course]
  end

  def require_admin
    user = current_user

    return true if user.has_role?(:admin)

    redirect_to root_path, alert: 'Permission denied'
  end
end
