Rails.application.routes.draw do
  # For details on the DSL available within this file, see http://guides.rubyonrails.org/routing.html
  devise_for :users

  root 'courses#index'

  # User
  resources :users, only: [:show]

  # Nested route
  resources :courses, only: %i[index show] do
    # Challenge
    resources :challenges, only: %i[index show] do
      member do
        post 'submit' => :submit, as: 'submit_flag'
      end
    end
  end

  namespace :admin do
    root 'courses#index'

    # Nested route
    resources :courses do
      # Challenge
      resources :challenges do
        member do
          post 'submit' => :submit, as: 'submit_flag'
        end
      end
    end
  end
end
