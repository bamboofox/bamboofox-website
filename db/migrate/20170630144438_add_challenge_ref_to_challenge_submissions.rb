class AddChallengeRefToChallengeSubmissions < ActiveRecord::Migration[5.1]
  def change
    add_reference :challenge_submissions, :challenge, foreign_key: true
  end
end
