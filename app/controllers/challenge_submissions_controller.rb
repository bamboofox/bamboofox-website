class ChallengeSubmissionsController < ApplicationController
  load_and_authorize_resource :course

  def index
    # if request from /challenge_submissions
    if params[:course_id]
      add_course_breadcrumbs
      breadcrumb 'Challenge Submissions', [@course, :challenge_submissions]
      @challenge_submissions = Course.find(params[:course_id]).challenge_submissions
    else
      breadcrumb 'Challenge Submissions', challenge_submissions_path
      @challenge_submissions = ChallengeSubmission.all
    end
    @challenge_submissions = Kaminari.paginate_array(@challenge_submissions.reverse).page params[:page]
  end

  def show
    @challenge_submission = ChallengeSubmission.find(params[:id])
    add_course_breadcrumbs
    breadcrumb 'Challenge Submissions', [@course, :challenge_submissions]
    breadcrumb @challenge_submission.id.to_s, [@course, @challenge_submission]
  end
end
