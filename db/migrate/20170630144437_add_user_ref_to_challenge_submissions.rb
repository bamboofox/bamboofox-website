class AddUserRefToChallengeSubmissions < ActiveRecord::Migration[5.1]
  def change
    add_reference :challenge_submissions, :user, foreign_key: true
  end
end
