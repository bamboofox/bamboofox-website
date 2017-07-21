class Challenge < ApplicationRecord
  mount_uploaders :attachments, AttachmentUploader
  serialize :attachments, JSON
  belongs_to :course
  has_many :challenge_submissions, dependent: :destroy
  alias_attribute :submissions, :challenge_submissions
  has_many :users, through: :challenge_submissions
  validates :name, presence: true
  validates :flag, presence: true
  validates :point, numericality: { only_integer: true, greater_than: 0 }
  resourcify
end
