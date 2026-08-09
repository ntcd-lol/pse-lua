--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Full session cycle: initialize -> GAME_INITIALIZE -> round-trip -> deinitialize.
--- --==-==--

Logger.info("MAIN", "PSE SDK Lua %s session init", PSE.version)

local r = PSE.initialize()
if r ~= 0 and r ~= false then
    Logger.error("MAIN", "PSE.initialize() failed (%s) - is the game running?", tostring(r))
    return
end
Logger.info("MAIN", "buffers initialized: connected to game shared memory")

local mode = core.mock() and "MOCK (offline simulation)" or "LIVE"
Logger.info("MAIN", "session mode: %s", mode)

local okGame, errGame = pcall(PSE.game.initialize, PSE.game)
if okGame then
    Logger.info("MAIN", "game initialized: player moved to the SDK level")
else
    Logger.warn("MAIN", "PSE.game:initialize() skipped: %s", tostring(errGame))
end

local g = PSE.game:getGravity()
Logger.info("MAIN", "PSE.game:getGravity() = %s", tostring(g))

PSE.game:setCheatsEnabled(true)
Logger.info("MAIN", "cheats enabled      = %s", tostring(PSE.game:getCheatsEnabled()))

local loc = PSE.player:getPosition()
Logger.info("MAIN", "player location   = %.1f %.1f %.1f",
    loc[1] or 0, loc[2] or 0, loc[3] or 0)

local floor = PSE.createMeshObject()
    :geometry("FACE")
    :texture("FLOOR_WHITE")
    :scale(6, 6, 1)
    :position(0, 0, 0)
    :create()
Logger.info("MAIN", "marker floor created, guid = %s", tostring(floor.guid))

local door = PSE.createDoor()
    :position(0, 100, 0)
    :name("init_door")
    :create()
Logger.info("MAIN", "door created, guid = %s, state = %d", tostring(door.guid), door:getState())

Logger.info("MAIN", "connection OK - the game answered every command")

PSE.game:deinitialize()
PSE.deinitialize()
Logger.info("MAIN", "session closed")
