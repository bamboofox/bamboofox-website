class AddHintToChallenges < ActiveRecord::Migration[5.1]
  def change
    add_column :challenges, :hint, :text
  end
end
