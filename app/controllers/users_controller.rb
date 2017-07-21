class UsersController < ApplicationController
  load_and_authorize_resource

  def index
    @users = User.accessible_by(current_ability, :index).page params[:page]
  end

  def show; end
end
