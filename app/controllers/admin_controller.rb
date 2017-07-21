class AdminController < ApplicationController
  before_action :authenticate_user!
  before_action :require_admin
  layout 'admin'

  def require_admin
    user = current_user

    return true if user.has_role?(:admin)

    redirect_to root_path, alert: 'Permission denied'
  end
end
