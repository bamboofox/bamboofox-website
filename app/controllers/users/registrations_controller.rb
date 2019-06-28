class Users::RegistrationsController < Devise::RegistrationsController
  before_action :configure_sign_up_params, only: [:create]
  before_action :configure_account_update_params, only: [:update]

  # GET /resource/sign_up
  def new
    super
  end

  # POST /resource
  def create
    super
  end

  # GET /resource/edit
  def edit
    super
  end

  # PUT /resource
  def update
    super
  end

  # DELETE /resource
  # def destroy
  #   super
  # end

  # GET /resource/cancel
  # Forces the session data which is usually expired after sign
  # in to be expired now. This is useful if the user wants to
  # cancel oauth signing in/up in the middle of the process,
  # removing all OAuth session data.
  # def cancel
  #   super
  # end

  protected

  # If you have extra params to permit, append them to the sanitizer.
  def configure_sign_up_params
    devise_parameter_sanitizer.permit(:sign_up, keys: %i[avatar avatar_cache name])
  end

  # If you have extra params to permit, append them to the sanitizer.
  def configure_account_update_params
    params[:user].delete :current_password if params[:user][:password].blank?
    devise_parameter_sanitizer.permit(:account_update) do |user_params|
      user_params.permit(:email, :is_email_visible, :name, :avatar, :avatar_cache, identities_attributes: %i[id _destroy])
    end
  end

  def update_resource(resource, _params)
    if account_update_params[:password].present?
      resource.update_with_password(account_update_params)
    else
      resource.update_without_password(account_update_params)
    end
  end

  # The path used after sign up.
  # def after_sign_up_path_for(resource)
  #   super(resource)
  # end

  # The path used after sign up for inactive accounts.
  # def after_inactive_sign_up_path_for(resource)
  #   super(resource)
  # end
end
