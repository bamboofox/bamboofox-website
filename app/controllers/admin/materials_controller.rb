class Admin::MaterialsController < AdminController
  load_and_authorize_resource :course
  load_and_authorize_resource :material, through: :course
  before_action :add_admin_course_breadcrumbs
  before_action { breadcrumb 'Materials', [:admin, @course, :materials] }
  before_action -> { breadcrumb @material.name, [:admin, @course, @material] }, only: %i[show edit]

  def index; end

  def show; end

  def new; end

  def edit; end

  def create
    if @material.save
      redirect_to admin_course_material_path(@course, @material), notice: 'Material was successfully created!'
    else
      redirect_to new_admin_course_material_path(@course), alert: 'Some fields are empty or not valid...'
    end
  end

  def update
    if @material.update(material_params)
      redirect_to admin_course_material_path(@course, @material), notice: 'Material was successfully updated!'
    else
      render :edit
    end
  end

  def destroy
    @material.destroy
    redirect_to admin_course_materials_path(@course), notice: "The material #{@material.name} has been deleted."
  end

  private

  def material_params
    params.require(:material).permit(:name, :is_external, :url, :attachment)
  end
end
