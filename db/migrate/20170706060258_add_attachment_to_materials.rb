class AddAttachmentToMaterials < ActiveRecord::Migration[5.1]
  def change
    add_column :materials, :attachment, :string
  end
end
