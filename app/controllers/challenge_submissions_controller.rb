class ChallengeSubmissionsController < ApplicationController
  load_and_authorize_resource :course

  def index
    # if request from /challenge_submissions
    if params[:course_id]
      @challenge_submissions = Course.find(params[:course_id]).challenge_submissions
    else
      @challenge_submissions = ChallengeSubmission.all
    end
    @challenge_submissions = Kaminari.paginate_array(@challenge_submissions.reverse).page params[:page]
  end

  def show
    @challenge_submission = ChallengeSubmission.find(params[:id])
  end
end
