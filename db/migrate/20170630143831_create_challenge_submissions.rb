class CreateChallengeSubmissions < ActiveRecord::Migration[5.1]
  def change
    create_table :challenge_submissions do |t|

      t.timestamps
    end
  end
end
