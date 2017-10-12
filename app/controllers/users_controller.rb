class UsersController < ApplicationController
  load_and_authorize_resource
  before_action { breadcrumb 'Users', users_path }
  before_action -> { breadcrumb @user.name, @user }, only: [:show]
  def index
    @users = User.accessible_by(current_ability, :index).page params[:page]
  end

  def show; end
end
