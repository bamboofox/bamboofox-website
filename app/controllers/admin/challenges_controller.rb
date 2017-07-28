class Admin::ChallengesController < AdminController
  load_and_authorize_resource :course
  load_and_authorize_resource :challenge, through: :course

  def index; end

  def show
    @submissions = @challenge.submissions.order('updated_at ASC')
  end

  def new; end

  def edit; end

  def create
    if @challenge.save
      redirect_to admin_course_challenge_path(@course, @challenge), notice: 'Challenge was successfully created!'
    else
      redirect_to new_admin_course_challenge_path(@course), alert: 'Some field are empty or not valid...'
    end
  end

  def update
    if @challenge.update(challenge_params)
      redirect_to admin_course_challenge_path(@course, @challenge), notice: 'Challenge was successfully updated!'
    else
      render :edit
    end
  end

  def destroy
    @challenge.destroy
    redirect_to admin_course_challenges_path(@course), alert: 'Challenge was successfully deleted!'
  end

  def submit
    if @challenge.flag != params[:flag]
      redirect_to admin_course_challenge_path(@course, @challenge), alert: 'Your flag is wrong! :('
    elsif @challenge.challenge_submissions.build(user_id: current_user.id, course_id: @challenge.course_id).save
      redirect_to admin_course_challenge_path(@course, @challenge), notice: 'Congratulation! You got it. :)'
    else
      redirect_to admin_course_challenge_path(@course, @challenge), alert: 'You have already solved this problem. :p'
    end
  end

  private

  def challenge_params
    params.require(:challenge).permit(:name, :description, :hint, :point, :flag, attachments: [])
  end
end
