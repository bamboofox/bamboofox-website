class Material < ApplicationRecord
  mount_uploader :attachment, AttachmentUploader
  belongs_to :course
end
