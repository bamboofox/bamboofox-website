class AddIsEmailVisibleToUsers < ActiveRecord::Migration[5.2]
  def change
    add_column :users, :is_email_visible, :boolean
  end
end
