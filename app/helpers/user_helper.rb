module UserHelper
  def icon_avatar(user, options = {})
    image_tag user.avatar.icon.url, options
  end

  def profile_avatar(user, options = {})
    image_tag user.avatar.profile.url, options
  end
end
