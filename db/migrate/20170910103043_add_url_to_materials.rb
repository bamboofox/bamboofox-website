class AddUrlToMaterials < ActiveRecord::Migration[5.1]
  def change
    add_column :materials, :url, :string
  end
end
