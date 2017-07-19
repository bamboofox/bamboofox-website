class Users::OmniauthCallbacksController < Devise::OmniauthCallbacksController
  # You should configure your model like this:
  # devise :omniauthable, omniauth_providers: [:twitter]

  def all
    if user_signed_in?
      link_omniauth
    else
      login_omniauth
    end
  end

  alias facebook all
  alias github all
  alias google_oauth2 all
  alias nctu all

  def failure
    set_flash_message! :alert, :failure, kind: 'Oauth', reason: 'Login process terminated!'
    redirect_to new_user_session_path
  end

  protected

  def link_omniauth
    if current_user.link_omniauth(request.env['omniauth.auth']).save
      set_flash_message! :notice, :success, kind: 'Oauth'
      sign_in_and_redirect current_user, event: :authentication
    else
      set_flash_message! :alert, :failure, kind: 'Oauth', reason: 'You already link account'
      redirect_to new_user_session_path
    end
  end

  def login_omniauth
    user = User.from_omniauth(request.env['omniauth.auth'])
    # Login
    if user.persisted?
      set_flash_message! :notice, :success, kind: 'Oauth'
      sign_in_and_redirect user, event: :authentication
    elsif user.save
      # Create user account using oauth and login
      set_flash_message! :notice, :success, kind: 'Oauth'
      sign_in_and_redirect user, event: :authentication
    else
      # User account already sign up
      set_flash_message! :alert, :failure, kind: 'Oauth', reason: 'You already have account'
      redirect_to new_user_session_path
    end
  end
  # You should also create an action method in this controller like this:
  # def twitter
  # end

  # More info at:
  # https://github.com/plataformatec/devise#omniauth

  # GET|POST /resource/auth/twitter
  # def passthru
  #   super
  # end

  # GET|POST /users/auth/twitter/callback

  # protected

  # The path used when OmniAuth fails
  # def after_omniauth_failure_path_for(scope)
  #   super(scope)
  # end
end
