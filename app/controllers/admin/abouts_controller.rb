class Admin::AboutsController < AdminController
  before_action :set_about
  authorize_resource

  def show; end

  def edit; end

  def update
    if @about.update(about_params)
      redirect_to admin_about_path, notice: 'About was successfully updated.'
    else
      render :edit
    end
  end

  private

  def set_about
    @about = About.first_or_create(content: '')
  end

  def about_params
    params.require(:about).permit(:content)
  end
end
