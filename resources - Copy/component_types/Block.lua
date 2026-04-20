Block = {
	transform = nil,

	OnStart = function(self)
		self.transform = self.actor:GetComponent("Transform")
		blocking_map[Round(self.transform.x) .. "," .. Round(self.transform.y)] = true
	end
}
