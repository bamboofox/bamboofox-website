class AddCourseRefToMaterials < ActiveRecord::Migration[5.1]
  def change
    add_reference :materials, :course, foreign_key: true
  end
end
