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

  def last_challenge_submission_time
    @last_challenge_submission_time ||= challenge_submissions.last&.created_at || Time.current
  end

  def total_point(course_id = nil)
    if course_id
      @challenges = challenges.where(course_id: course_id)
    else
      @challenges = challenges
    end
    @total_point ||= @challenges.sum(&:point)
  end
end
