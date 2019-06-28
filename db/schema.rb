# This file is auto-generated from the current state of the database. Instead
# of editing this file, please use the migrations feature of Active Record to
# incrementally modify your database, and then regenerate this schema definition.
#
# Note that this schema.rb definition is the authoritative source for your
# database schema. If you need to create the application database on another
# system, you should be using db:schema:load, not running all the migrations
# from scratch. The latter is a flawed and unsustainable approach (the more migrations
# you'll amass, the slower it'll run and the greater likelihood for issues).
#
# It's strongly recommended that you check this file into your version control system.

ActiveRecord::Schema.define(version: 20_190_628_140_543) do
  create_table 'abouts', force: :cascade do |t|
    t.text 'content'
    t.datetime 'created_at', null: false
    t.datetime 'updated_at', null: false
  end

  create_table 'challenge_submissions', force: :cascade do |t|
    t.datetime 'created_at', null: false
    t.datetime 'updated_at', null: false
    t.integer 'course_id'
    t.integer 'user_id'
    t.integer 'challenge_id'
    t.index ['challenge_id'], name: 'index_challenge_submissions_on_challenge_id'
    t.index ['course_id'], name: 'index_challenge_submissions_on_course_id'
    t.index ['user_id'], name: 'index_challenge_submissions_on_user_id'
  end

  create_table 'challenges', force: :cascade do |t|
    t.string 'name'
    t.text 'description'
    t.string 'flag'
    t.integer 'point'
    t.datetime 'created_at', null: false
    t.datetime 'updated_at', null: false
    t.integer 'course_id'
    t.string 'attachments'
    t.text 'hint'
    t.index ['course_id'], name: 'index_challenges_on_course_id'
  end

  create_table 'courses', force: :cascade do |t|
    t.text 'name'
    t.datetime 'created_at', null: false
    t.datetime 'updated_at', null: false
  end

  create_table 'identities', force: :cascade do |t|
    t.integer 'user_id'
    t.string 'provider'
    t.string 'uid'
    t.datetime 'created_at', null: false
    t.datetime 'updated_at', null: false
    t.index ['user_id'], name: 'index_identities_on_user_id'
  end

  create_table 'materials', force: :cascade do |t|
    t.string 'name'
    t.datetime 'created_at', null: false
    t.datetime 'updated_at', null: false
    t.string 'attachment'
    t.integer 'course_id'
    t.string 'url'
    t.boolean 'is_external'
    t.index ['course_id'], name: 'index_materials_on_course_id'
  end

  create_table 'roles', force: :cascade do |t|
    t.string 'name'
    t.string 'resource_type'
    t.integer 'resource_id'
    t.datetime 'created_at', null: false
    t.datetime 'updated_at', null: false
    t.index %w[name resource_type resource_id], name: 'index_roles_on_name_and_resource_type_and_resource_id'
    t.index ['name'], name: 'index_roles_on_name'
    t.index %w[resource_type resource_id], name: 'index_roles_on_resource_type_and_resource_id'
  end

  create_table 'users', force: :cascade do |t|
    t.string 'email', default: '', null: false
    t.string 'encrypted_password', default: '', null: false
    t.string 'reset_password_token'
    t.datetime 'reset_password_sent_at'
    t.datetime 'remember_created_at'
    t.integer 'sign_in_count', default: 0, null: false
    t.datetime 'current_sign_in_at'
    t.datetime 'last_sign_in_at'
    t.string 'current_sign_in_ip'
    t.string 'last_sign_in_ip'
    t.string 'confirmation_token'
    t.datetime 'confirmed_at'
    t.datetime 'confirmation_sent_at'
    t.string 'unconfirmed_email'
    t.datetime 'created_at', null: false
    t.datetime 'updated_at', null: false
    t.string 'avatar'
    t.string 'name'
    t.boolean 'is_email_visible'
    t.index ['email'], name: 'index_users_on_email', unique: true
    t.index ['reset_password_token'], name: 'index_users_on_reset_password_token', unique: true
  end

  create_table 'users_roles', id: false, force: :cascade do |t|
    t.integer 'user_id'
    t.integer 'role_id'
    t.index ['role_id'], name: 'index_users_roles_on_role_id'
    t.index %w[user_id role_id], name: 'index_users_roles_on_user_id_and_role_id'
    t.index ['user_id'], name: 'index_users_roles_on_user_id'
  end
end
