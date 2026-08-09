--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Как это работает:
--- * Точка входа Lua-SDK: вызывается хостом через require("sdk.init").
--- * Грузит sdk.api (и вместе с ним весь SDK), выставляет глобальные
---   PSE / pse (таблица API) и Logger (удобный логгер).
--- * Инициализирует логгер и возвращает PSE вызывающей стороне.
--- --==-==--

local PSE = require("sdk.api")

_G.PSE = PSE
_G.pse = PSE
_G.Logger = PSE.Logger

PSE.Logger.init()

return PSE
