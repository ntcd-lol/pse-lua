--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Button -> door/lamp via the ELEMENT_CHANGED callback (Callback Ping).
--- The session stays alive (PSE.run without timeout) so the player has time
--- to reach the button.
--- Run (live):  bin\pse_lua.exe examples\button.lua
--- Run (mock):  bin\pse_lua.exe --mock examples\button.lua
--- --==-==--

Logger.info("MAIN", "button + door demo")

local live = PSE.initialize()
PSE.setCheatsEnabled(true)

local floor = PSE.createMeshObject()
    :geometry("FACE")
    :texture("FLOOR_WHITE")
    :scale(6, 6, 1)
    :position(0, 0, 0)
    :create()

local door = PSE.createDoor()
    :position(0, 100, 0)
    :name("door")
    :register(0, 1)
    :create()
Logger.info("MAIN", "door guid = %s, state = %d", tostring(door.guid), door:getState())

local lamp = PSE.createLamp()
    :position(60, -100, 0)
    :name("lamp")
    :register(1, PSE.palette.green)
    :register(2, PSE.palette.red)
    :create()
Logger.info("LAMP", "lamp guid = %s", tostring(lamp.guid))

local button = PSE.createButton()
    :position(0, -100, 0)
    :name("button")
    :register(1, PSE.palette.orange)
    :onChange(function(state, guid)
        Logger.info("BUTTON", "callback fired! state=%d guid=%s", state, tostring(guid))
        door:setState(state)
        lamp:setState(state)
    end)
    :create()
Logger.info("BUTTON", "button guid = %s", tostring(button.guid))

if core.mock() then
    Logger.info("MAIN", "mock mode: simulating 3 button presses...")
    for i = 1, 3 do
        PSE.mock.emit("button", i % 2)
        PSE.pollAll()
        Logger.info("MAIN", "after press %d door state = %d, lamp state = %d",
            i, door:getState(), lamp:getState())
    end
else
    Logger.info("MAIN", "live mode: walk to the button and press it (Ctrl+C to stop)")
    PSE.run()
end

Logger.info("MAIN", "done")
