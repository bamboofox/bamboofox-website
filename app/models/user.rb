class User < ApplicationRecord
  mount_uploader :avatar, AvatarUploader
  has_many :challenge_submissions
  has_many :challenges, through: :challenge_submissions
  resourcify
  rolify
  # Include default devise modules. Others available are:
  # :confirmable, :lockable, :timeoutable and :omniauthable
  devise :database_authenticatable, :registerable,
         :recoverable, :rememberable, :trackable, :validatable, :confirmable
end
