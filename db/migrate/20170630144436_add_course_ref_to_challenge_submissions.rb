class AddCourseRefToChallengeSubmissions < ActiveRecord::Migration[5.1]
  def change
    add_reference :challenge_submissions, :course, foreign_key: true
  end
end
