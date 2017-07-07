module OmniauthHelper
  def omniauth_buttons(providers)
    sanitize(providers.map do |provider|
      case provider
      when :facebook
        link_to omniauth_authorize_path(resource_name, provider), class: 'ui facebook button' do
          tag :i, class: 'facebook icon'
          'Facebook'
        end
      when :github
        link_to omniauth_authorize_path(resource_name, provider), class: 'ui github button' do
          tag :i, class: 'github icon'
          'Github'
        end
      when :google_oauth2
        link_to omniauth_authorize_path(resource_name, provider), class: 'ui google plus button' do
          tag :i, class: 'google plus icon'
          'Google'
        end
      end
    end.join)
  end
end
