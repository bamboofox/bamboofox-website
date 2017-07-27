module OmniauthHelper
  def omniauth_buttons(providers)
    sanitize(providers.map do |provider|
      case provider
      when :facebook
        link_to omniauth_authorize_path(resource_name, provider), class: 'ui facebook button' do
          tag :i, class: 'facebook icon'
          provider.to_s.camelize
        end
      when :github
        link_to omniauth_authorize_path(resource_name, provider), class: 'ui github button' do
          tag :i, class: 'github icon'
          provider.to_s.camelize
        end
      when :google
        link_to omniauth_authorize_path(resource_name, provider), class: 'ui google plus button' do
          tag :i, class: 'google plus icon'
          provider.to_s.camelize
        end
      when :nctu
        link_to omniauth_authorize_path(resource_name, provider), class: 'ui nctu button' do
          tag :i, class: 'nctu icon'
          provider.to_s.camelize
        end
      end
    end.join)
  end
end
