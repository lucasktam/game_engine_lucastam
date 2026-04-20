GameManager = {

	OnStart = function(self)

		Round = function(v)
			return math.floor(v + 0.5)
		end

		blocking_map = {}
		tile_map = {}

		local stage = next_stage or 1
		local map_file_path = "resources/data/map" .. stage .. ".lua"
		local map = dofile(map_file_path)

		for _, layer in ipairs(map.layers) do
			if layer.type == "objectgroup" then
				for _, obj in ipairs(layer.objects) do
					local x = obj.x or 0
					local y = obj.y or 0
					local gid = obj.gid - 1 -- Lua is one-indexed, subtract 1 to match Tiled's ID system

					-- Normalize X and Y positions
					x = x / 100
					y = y / 100

					if gid == 15 then -- floor tile
						local new_tile = Actor.Instantiate("Tile")
						local new_tile_t = new_tile:GetComponent("Transform")
						new_tile_t.x = x
						new_tile_t.y = y
					end

					if gid == 5 then -- block
						local new_box = Actor.Instantiate("Block")
						local new_box_t = new_box:GetComponent("Transform")
						new_box_t.x = x
						new_box_t.y = y
					end

					if gid == 2 then -- player
						local new_player = Actor.Instantiate("Player")
						local new_player_t = new_player:GetComponent("Transform")
						new_player_t.x = x
						new_player_t.y = y
					end

					if gid == 9 then -- door
						local new_door = Actor.Instantiate("Door")
						local new_door_t = new_door:GetComponent("Transform")
						new_door_t.x = x
						new_door_t.y = y

						-- Read stage from properties array
						local door_stage = 1
						if obj.properties then
							for _, prop in ipairs(obj.properties) do
								if prop.name == "stage" then
									door_stage = prop.value
									break
								end
							end
						end
						new_door:GetComponent("AdvanceStageOnTouch").stage = door_stage
					end
				end
			end
		end

	end
}
