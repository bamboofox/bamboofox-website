class Admin::RanksController < AdminController
  load_and_authorize_resource :course

  def show
    # if request from /course/:id/rank
    if params[:course_id]
      add_admin_course_breadcrumbs
      breadcrumb 'Rank', [:admin, @course, :rank]
      @users = User.find_each.sort_by do |user|
        [user.total_point(params[:course_id]), -user.last_challenge_submission_time.to_i]
      end.reverse
    # if request from /rank
    else
      breadcrumb 'Rank', admin_rank_path
      @users = User.find_each.sort_by do |user|
        [user.total_point, -user.last_challenge_submission_time.to_i]
      end.reverse
    end
    @users = Kaminari.paginate_array(@users).page params[:page]
  end
end
