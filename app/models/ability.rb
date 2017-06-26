class Ability
  include CanCan::Ability

  def initialize(user)
    user ||= User.new
    can :manage, user
    can :read, Course
    can :manage, :all if user.has_role? :admin
  end
end
