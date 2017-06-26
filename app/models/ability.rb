class Ability
  include CanCan::Ability

  def initialize(user)
    user ||= User.new
    can :manage, user
  end
end
