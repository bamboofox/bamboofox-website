Rails.application.routes.draw do
  # For details on the DSL available within this file, see http://guides.rubyonrails.org/routing.html
  devise_for :users, controllers: { registrations: 'users/registrations', omniauth_callbacks: 'users/omniauth_callbacks' }

  root 'courses#index'

  # User
  resources :users, only: %i[index show]

  # Rank
  resource :rank, only: [:show]

  # Nested route
  resources :courses, only: %i[index show] do
    # Challenge
    resources :challenges, only: %i[index show] do
      member do
        post 'submit' => :submit, as: 'submit_flag'
      end
    end

    # Material
    resources :materials, only: %i[index show]

    # Rank
    resource :rank, only: [:show]
  end

  namespace :admin do
    root 'courses#index'

    # Rank
    resource :rank, only: [:show]

    # Nested route
    resources :courses do
      # Challenge
      resources :challenges do
        member do
          post 'submit' => :submit, as: 'submit_flag'
        end
      end

      # Material
      resources :materials

      # Rank
      resource :rank, only: [:show]
    end
  end
end
