class AddIsExternalToMaterials < ActiveRecord::Migration[5.1]
  def change
    add_column :materials, :is_external, :boolean
  end
end
