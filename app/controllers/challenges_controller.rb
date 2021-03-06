class ChallengesController < ApplicationController
  load_and_authorize_resource :course
  load_and_authorize_resource :challenge, through: :course
  before_action :require_login!, only: [:submit]
  before_action :add_course_breadcrumbs
  before_action -> { breadcrumb 'Challenges', [@course, :challenges] }
  before_action -> { breadcrumb @challenge.name, [@course, @challenge] }, only: [:show]

  def index; end

  def show
    @submissions = @challenge.submissions.order('updated_at ASC')
  end

  def submit
    if @challenge.flag != params[:flag]
      redirect_to course_challenge_path(@course, @challenge), alert: 'Your flag is wrong! :('
    elsif @challenge.challenge_submissions.build(user_id: current_user.id, course_id: @challenge.course_id).save
      redirect_to course_challenge_path(@course, @challenge), notice: 'Congratulation! You got it. :)'
    else
      redirect_to course_challenge_path(@course, @challenge), alert: 'You have already solved this problem. :p'
    end
  end
end
