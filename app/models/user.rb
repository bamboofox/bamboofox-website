class User < ApplicationRecord
  mount_uploader :avatar, AvatarUploader
  has_many :challenge_submissions
  has_many :challenges, through: :challenge_submissions
  has_many :identities
  accepts_nested_attributes_for :identities, allow_destroy: true
  validates :name, presence: true
  validates :email, presence: true
  resourcify
  rolify
  # Include default devise modules. Others available are:
  # :confirmable, :lockable, :timeoutable and :omniauthable
  devise :database_authenticatable, :registerable,
         :recoverable, :rememberable, :trackable,
         :validatable, :confirmable, :omniauthable,
         omniauth_providers: %i[facebook github google_oauth2 nctu]

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

  def self.from_omniauth(auth)
    joins(:identities).where(identities: { provider: auth.provider, uid: auth.uid }).first_or_initialize do |user|
      case auth.provider
      when 'nctu'
        user.name = auth.extra.raw_info.username
        user.email = auth.extra.raw_info.email
      else
        user.name = auth.info.name
        user.email = auth.info.email
        user.remote_avatar_url = auth.info.image
      end
      user.password = Devise.friendly_token[0, 20]
      user.identities.build(provider: auth.provider, uid: auth.uid)
      user.skip_confirmation!
    end
  end

  def link_omniauth(auth)
    identities.build(provider: auth.provider, uid: auth.uid)
  end
end
