turnSpeed = 80
MoveSpeed = 10

local transitionTime = 5
local time = 0
local currentState = "None"
function BeginPlay()
    print("Begin")
    addAnimState("Walk","Armature|Walking")
    addAnimState("Jump","Armature|Jumping")
    changeAnimState("Walk", 0, true)
    currentState = "Walk"
    startAnim()
end

function EndPlay()
    print("[EndPlay]")
    stopAnim()
end

function OnOverlap(OtherActor)
end

function InitializeLua()
    controller("W", OnPressW)    
    controller("S", OnPressS)
    controller("A", OnPressA)
    controller("D", OnPressD)
    controller("F", OnPressF)
    controller("G", OnPressG)
end

function OnPressW(dt)
    local currentPos = actor.Location
    currentPos = currentPos + actor:Forward() * dt * MoveSpeed
    actor.Location = currentPos
    changeAnimState("Walk", 0.2, true)
end

function OnPressS(dt)
    local currentPos = actor.Location
    currentPos = currentPos - actor:Forward() * dt * MoveSpeed
    actor.Location = currentPos    
    changeAnimState("Jump", 0.2, false)
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

function OnPressF(dt)
    changeAnimState("Jump", 0, false)
end

function OnPressG(dt)
    changeAnimState("Walk", 0, true)
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
