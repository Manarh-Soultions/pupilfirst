module Targets
  class AutoVerificationService
    def initialize(target, founder)
      @target = target
      @founder = founder
    end

    def auto_verify
      @target.timeline_events.create!(
        founder: @founder,
        startup: @founder.startup,
        description: description,
        timeline_event_type: team_update,
        event_on: Time.zone.now,
        iteration: @founder.startup.iteration,
        status: TimelineEvent::STATUS_VERIFIED
      )
    end

    private

    def description
      "Target '#{@target.title}' was auto-verified"
    end

    def team_update
      TimelineEventType.find_by(key: TimelineEventType::TYPE_TEAM_UPDATE)
    end
  end
end