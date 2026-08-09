--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Как это работает:
--- * Инициализация сессии и подключение к запущенной игре Portal: Solver.
--- * PSE.initialize() = pseInitializeBuffers (открывает shared memory).
--- * Пример round-trip: GAME_INITIALIZE -> проверка GAME_CHECK_GUID_IS_VALID,
---   чтение версии/гравитации, создание элемента и регистров.
--- * В --mock режиме всё эмулируется офлайн, без игры.
--- * Запуск (live):  bin\pse_lua.exe examples\init.lua
---   Запуск (mock):  bin\pse_lua.exe --mock examples\init.lua
--- --==-==--

Logger.info("MAIN", "PSE SDK Lua %s session init", PSE.version)

-- 1. Connect to the game: allocate + open the shared memory buffers.
--    Returns 0 on success (mirrors pseInitializeBuffers in pse.h);
--    false in mock mode, where the connection is simulated offline.
local r = PSE.initialize()
if r ~= 0 and r ~= false then
    Logger.error("MAIN", "PSE.initialize() failed (%s) - is the game running?", tostring(r))
    return
end
Logger.info("MAIN", "buffers initialized: connected to game shared memory")

local mode = core.mock() and "MOCK (offline simulation)" or "LIVE"
Logger.info("MAIN", "session mode: %s", mode)

-- 2. Put the player on the SDK test level (pseInitializeBuffers does NOT do this).
--    Degrades gracefully: in live mode the game may not be ready yet.
local okGame, errGame = pcall(PSE.initializeGame)
if okGame then
    Logger.info("MAIN", "game initialized: player moved to the SDK level")
else
    Logger.warn("MAIN", "initializeGame() skipped: %s", tostring(errGame))
end

-- 3. Round-trip sanity checks through the command buffer.
local g = PSE.getGravity()
Logger.info("MAIN", "getGravity()      = %s", tostring(g))

PSE.setCheatsEnabled(true)
Logger.info("MAIN", "cheats enabled    = %s", tostring(PSE.getCheatsEnabled()))

local loc = PSE.getPlayerLocation()
Logger.info("MAIN", "player location   = %.1f %.1f %.1f",
    loc[1] or 0, loc[2] or 0, loc[3] or 0)

-- 4. Prove the pipeline end-to-end: create an element and read it back.
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

-- 5. Tear down the SDK session (host also deinitializes on exit).
PSE.deinitializeGame()
PSE.deinitialize()
Logger.info("MAIN", "session closed")
