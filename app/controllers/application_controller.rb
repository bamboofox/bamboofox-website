class ApplicationController < ActionController::Base
  protect_from_forgery with: :exception
  def authenticate_user!
    unless user_signed_in?
      redirect_to new_user_session_path, notice: 'You need to sign in or sign up before continuing.'
    end
  end
end
