AdvanceStageOnTouch = {

	stage = 1,

	OnStart = function(self)
		self.transform = self.actor:GetComponent("Transform")
	end,

	OnUpdate = function(self)
		local player = Actor.Find("Player")
		if player == nil then return end

		local player_t = player:GetComponent("Transform")
		local dx = self.transform.x - player_t.x
		local dy = self.transform.y - player_t.y
		local distance = math.sqrt(dx * dx + dy * dy)

		if distance < 0.2 then
			next_stage = self.stage
			Scene.Load("gameplay")
		end
	end
}
