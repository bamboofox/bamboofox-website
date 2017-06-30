class AddCourseRefToChallenges < ActiveRecord::Migration[5.1]
  def change
    add_reference :challenges, :course, foreign_key: true
  end
end
