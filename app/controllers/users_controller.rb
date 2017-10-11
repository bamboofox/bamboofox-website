class UsersController < ApplicationController
  load_and_authorize_resource

  def index
    @users = User.accessible_by(current_ability, :index).page params[:page]
    breadcrumb 'Users', users_path
  end

  def show
    breadcrumb 'Users', users_path
    breadcrumb @user.name, @user
  end
end
