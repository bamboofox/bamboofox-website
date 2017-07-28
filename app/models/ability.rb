class Ability
  include CanCan::Ability

  def initialize(user)
    user ||= User.new
    can :read, About
    can :read, User
    can :read, Course
    can %i[read submit], Challenge
    can :read, Material
    can :read, ChallengeSubmission
    can :manage, user
    can :manage, :all if user.has_role? :admin
  end
end
