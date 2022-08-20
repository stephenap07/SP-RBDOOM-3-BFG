WeaponUnarmed = {
	EVENTS_LISTENERS = {}
}

function WeaponUnarmed:Construct()
	sys:println("Constructing " .. self:getName())
	-- Just holster the unarmed weapon immediately
	self:weaponHolstered()
end

function WeaponUnarmed:Destroy()
end