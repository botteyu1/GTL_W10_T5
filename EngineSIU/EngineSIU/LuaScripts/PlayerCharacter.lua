turnSpeed = 80
MoveSpeed = 10

local transitionTime = 2
local time = 0
local currentState = "None"
function BeginPlay()
    print("Begin")
    addAnimState("Walk","Armature|Walking2")
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
        time = time + dt

    if time >= transitionTime then
        if currentState == "Walk" then
            changeAnimState("Jump", 0.2, false)
            currentState = "Jump"
            
        else
            changeAnimState("Walk", 0.2, true)
            currentState = "Walk"
        end
        time = 0
        print(currentState)
    end

end

function BeginOverlap()
end

function EndOverlap()
end

function StartAnimNotify(NotifyName)
    -- if(NotifyName == "LuaJump") then
    --     actor.Location = FVector(0,0,0)
    -- end
    
end

function TickAnimNotify(NotifyName, DeltaTime)
    -- if(NotifyName == "LuaJump") then
    --     actor.Location = actor.Location + FVector(0,0,0.2)
    -- end
end

function EndAnimNotify(NotifyName)
    -- if(NotifyName == "LuaJump") then
    --     actor.Location = FVector(0,0,0)
    -- end
end
