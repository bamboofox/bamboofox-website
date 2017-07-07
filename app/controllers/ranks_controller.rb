class RanksController < ApplicationController
  load_and_authorize_resource :course

  def show
    # if request from /course/:id/rank
    if params[:course_id]
      @users = User.find_each.sort_by do |user|
        [user.total_point(params[:course_id]), -user.last_challenge_submission_time.to_i]
      end.reverse
    # if request from /rank
    else
      @users = User.find_each.sort_by do |user|
        [user.total_point, -user.last_challenge_submission_time.to_i]
      end.reverse
    end
  end
end
