class Ability
  include CanCan::Ability

  def initialize(user)
    user ||= User.new
    can :read, User
    can :read, Course
    can %i[read submit], Challenge
    can :read, Material
    can :manage, user
    can :manage, :all if user.has_role? :admin
  end
end
