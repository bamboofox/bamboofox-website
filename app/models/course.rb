class Course < ApplicationRecord
  has_many :challenges, dependent: :destroy
  has_many :challenge_submissions
  validates :name, presence: true
  resourcify
end
