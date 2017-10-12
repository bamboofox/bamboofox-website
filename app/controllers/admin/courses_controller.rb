class Admin::CoursesController < AdminController
  load_and_authorize_resource
  before_action { breadcrumb 'Courses', :admin_courses_path }
  before_action -> { breadcrumb @course.name, [:admin, @course] }, only: %i[show edit]

  def index; end

  def show; end

  def new; end

  def edit; end

  def create
    if @course.save
      redirect_to admin_course_path(@course), notice: 'Course was successfully created!'
    else
      redirect_to new_admin_course_path, alert: 'Some fields are empty or not valid...'
    end
  end

  def update
    if @course.update(course_params)
      redirect_to admin_course_path(@course), notice: 'Course was successfully updated!'
    else
      render :edit
    end
  end

  def destroy
    @course.destroy
    redirect_to admin_courses_path, alert: 'Course was successfully deleted!'
  end

  private

  def course_params
    params.require(:course).permit(:name)
  end
end
