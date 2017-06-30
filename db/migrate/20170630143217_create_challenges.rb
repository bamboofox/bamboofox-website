class CreateChallenges < ActiveRecord::Migration[5.1]
  def change
    create_table :challenges do |t|
      t.string :name
      t.text :description
      t.string :flag
      t.integer :point

      t.timestamps
    end
  end
end
