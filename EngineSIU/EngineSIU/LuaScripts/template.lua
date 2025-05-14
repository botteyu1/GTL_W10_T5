turnSpeed = 80
MoveSpeed = 10

function BeginPlay()
    print("Begin")
end

function EndPlay()
    print("[EndPlay]")
end

function OnOverlap(OtherActor)
end

function InitializeLua()
    controller("W", OnPressW)    
    controller("S", OnPressS)
    controller("A", OnPressA)
    controller("D", OnPressD)
end

function OnPressW(dt)
    local currentPos = actor.Location
    currentPos = currentPos + actor:Forward() * dt * MoveSpeed
    actor.Location = currentPos
end

function OnPressS(dt)
    local currentPos = actor.Location
    currentPos = currentPos - actor:Forward() * dt * MoveSpeed
    actor.Location = currentPos    
end

function OnPressA(dt)
    local rot = actor.Rotator
    rot.Yaw = rot.Yaw - turnSpeed * dt
    actor.Rotator = rot
end

function OnPressD(dt)
    local rot = actor.Rotator
    rot.Yaw = rot.Yaw + turnSpeed * dt
    actor.Rotator = rot
end

function Tick(dt)
    -- actor:SetLocation(newPos)
    -- actor:SetLocation(FVector(200, 0, 0))
    -- UE_LOG("Error", "actor:SetLocation")

    -- actor.Location = FVector(0, 0, 0)
    -- actor.Location.X = actor.Location.X + dt
end

function BeginOverlap()
end

function EndOverlap()
end

function StartAnimNotify(NotifyName)
    if(NotifyName == "LuaJump") then
        actor.Location = FVector(0,0,0)
    end
    
end

function TickAnimNotify(NotifyName, DeltaTime)
    if(NotifyName == "LuaJump") then
        actor.Location = actor.Location + FVector(0,0,0.2)
    end
end

function EndAnimNotify(NotifyName)
    if(NotifyName == "LuaJump") then
        actor.Location = FVector(0,0,0)
    end
end
