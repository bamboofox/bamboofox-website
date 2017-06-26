# This file should contain all the record creation needed to seed the database with its default values.
# The data can then be loaded with the rails db:seed command (or created alongside the database with db:setup).
#
# Examples:
#
#   movies = Movie.create([{ name: 'Star Wars' }, { name: 'Lord of the Rings' }])
#   Character.create(name: 'Luke', movie: movies.first)

# Run bin/rake db:seed to load data
# Generate data to fill the layout
json = ActiveSupport::JSON.decode(File.read('db/seeds.json'))

json['users'].each do |user|
  user = User.new(user)
  user.password_confirmation = user.password
  user.skip_confirmation!
  user.save!
end

json['courses'].each do |course|
  Course.create!(course)
end

# Add roles

user = User.find(1)
user.add_role :admin
