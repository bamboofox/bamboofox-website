Rails.application.routes.draw do
  # For details on the DSL available within this file, see http://guides.rubyonrails.org/routing.html
  devise_for :users

  root 'courses#index'
  resources :courses, only: %i[index show]

  namespace :admin do
    root 'courses#index'
    resources :courses
  end
end
