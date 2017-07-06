class AddAttachmentsToChallenges < ActiveRecord::Migration[5.1]
  def change
    add_column :challenges, :attachments, :string
  end
end
