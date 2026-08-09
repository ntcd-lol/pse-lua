--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Как это работает:
--- * Классический первый скрипт: создаёт пол (dynamic mesh FACE/FLOOR_WHITE)
---   и дверь (class DOOR), затем round-trip проверяет состояние/позицию.
--- * Демонстрирует fluent-стиль PSE.createMeshObject():geometry(...)
---   :texture(...):scale(...):position(...):create().
--- * Читает обратно state/transform/регистры созданных объектов.
--- * Запуск:  bin\pse_lua.exe --mock examples\hello.lua
--- --==-==--

Logger.info("MAIN", "hello, PSE SDK Lua!")

local live = PSE.initialize()
Logger.info("MAIN", "session mode: %s", live and "LIVE" or "MOCK")
PSE.setCheatsEnabled(true)
local floor = PSE.createMeshObject()
    :geometry("FACE")
    :texture("FLOOR_WHITE")
    :scale(8, 8, 1)
    :position(0, 0, -100)
    :create()
Logger.info("MAIN", "floor created, guid = %s", tostring(floor.guid))

local door = PSE.createDoor()
    :position(0, 60, -100)
    :state(0)
    :name("main_door")
    :create()
Logger.info("MAIN", "door created, guid = %s", tostring(door.guid))

Logger.info("MAIN", "door state = %d (open)", door:setState(1):getState())
Logger.info("MAIN", "door state = %d (closed)", door:setState(0):getState())

door:setPosition(100, 200, -100)
local t = door:getTransform()
Logger.info("MAIN", "door position = %s %s %s",
    tostring(t.location[1]), tostring(t.location[2]), tostring(t.location[3]))

door:setRegister(0, PSE.Registers.build("DOOR", 0, { A = 1, C = 0 }))
Logger.info("MAIN", "door flag register = 0x%08X (A=1)", door:getRegister(0))

Logger.info("MAIN", "done. try PSE.get('main_door'):getState() in the REPL")
