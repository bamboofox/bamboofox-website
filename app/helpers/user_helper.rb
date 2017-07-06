module UserHelper
  def icon_avatar(user, options = {})
    if user.avatar?
      image_tag user.avatar.icon.url, options
    else
      image_tag 'icon_avatar.jpg', options
    end
  end

  def profile_avatar(user, options = {})
    if user.avatar?
      image_tag user.avatar.profile.url, options
    else
      image_tag 'profile_avatar.jpg', options
    end
  end
end
