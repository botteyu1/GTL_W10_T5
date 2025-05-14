turnSpeed = 80
MoveSpeed = 100

local transitionTime = 2
local time = 0
local currentState = "None"
local currentIndex = 0
local animStates = {
    "Idle",
    "Walk",
    "Jump",
    "Punch"
}
function BeginPlay()
    print("Begin")
    addAnimState("Idle", "Armature|Idle")
    addAnimState("Walk","Armature|Walking2")
    addAnimState("Jump","Armature|Jumping")
    addAnimState("Punch", "Armature|Punch")
    changeAnimState("Idle", 0, true)
    currentState = "Idle"
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
    if currentState == "Punch" then 
        return
    end

    local currentPos = actor.Location
    currentPos = currentPos + actor:Forward() * dt * MoveSpeed
    actor.Location = currentPos
end

function OnPressS(dt)
    if currentState == "Punch" then 
        return
    end

    local currentPos = actor.Location
    currentPos = currentPos - actor:Forward() * dt * MoveSpeed
    actor.Location = currentPos    
end

function OnPressA(dt)
    if currentState == "Punch" then 
        return
    end

    local rot = actor.Rotator
    rot.Yaw = rot.Yaw - turnSpeed * dt
    actor.Rotator = rot
end

function OnPressD(dt)
    if currentState == "Punch" then 
        return
    end

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
    time = time + dt 

    local randomAnim = animStates[1]

    if time >= transitionTime then
        if #animStates > 1 then
            repeat
                local index = math.random(1, #animStates)
                randomAnim = animStates[index]
            until randomAnim ~= currentState  
        end

        changeAnimState(randomAnim, 0.2, true)

        currentState = randomAnim
        time = 0
        transitionTime = randomFloat(2, 5)  

        print("Changed to:", currentState)
    end
end

function BeginOverlap()
end

function EndOverlap()
end
PlayerLocation = FVector(0,0,0)
function StartAnimNotify(NotifyName)
    -- if(NotifyName == "LuaJump") then
    --     actor.Location = FVector(0,0,0)
    -- end
    if(NotifyName == "Teleport") then
        PlayerLocation = actor.Location
        actor.Location = actor.Location + FVector(50,0,0)
    end
end

function TickAnimNotify(NotifyName, DeltaTime)
    -- if(NotifyName == "LuaJump") then
    --     actor.Location = actor.Location + FVector(0,0,0.2)
    -- end
    if(NotifyName == "Teleport") then
        actor.Location = actor.Location + FVector(DeltaTime*100, 0, 0)
    end
end

function EndAnimNotify(NotifyName)
    -- if(NotifyName == "LuaJump") then
    --     actor.Location = FVector(0,0,0)
    -- end
    if(NotifyName == "Teleport") then
        actor.Location = PlayerLocation
    end
end

function randomFloat(min, max)
    return min + math.random() * (max - min)
end