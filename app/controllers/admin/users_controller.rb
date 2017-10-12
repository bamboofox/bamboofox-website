class Admin::UsersController < AdminController
  load_and_authorize_resource
  before_action { breadcrumb 'Users', admin_users_path }
  before_action -> { breadcrumb @user.name, [:admin, @user] }, only: %i[show edit]

  def index
    @users = User.accessible_by(current_ability, :index).page params[:page]
  end

  def show; end

  def new; end

  def edit; end

  def create
    @user = User.new(user_params)
    if @user.save
      redirect_to admin_user_path(@user), notice: 'User was successfully created!'
    else
      redirect_to new_admin_user_path, alert: 'Some fields are empty or not valid...'
    end
  end

  def update
    params[:user][:role_ids] ||= []
    if params[:user][:password].present?
      if @user.update_attributes(user_params)
        redirect_to admin_user_path(@user), notice: 'User was successfully updated!'
      else
        render :edit
      end
    elsif @user.update_without_password(user_params)
      redirect_to admin_user_path(@user), notice: 'User was successfully updated!'
    else
      render :edit
    end
  end

  def destroy
    @user.destroy
    redirect_to admin_users_path, alert: 'User was successfully deleted!'
  end

  private

  def user_params
    params.require(:user).permit(:name, :email, :password, :password_confirmation, :avatar, :avatar_cache, { role_ids: [] }, :confirmed_at)
  end
end
