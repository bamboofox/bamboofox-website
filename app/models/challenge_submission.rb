class ChallengeSubmission < ApplicationRecord
  belongs_to :course
  belongs_to :challenge
  belongs_to :user
  validates :user_id, uniqueness: { scope: :challenge_id }
  validates :challenge, presence: true
  validates :user, presence: true
  resourcify
end
