--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- SDK entry point (loaded by the host): loads sdk.api and exposes
--- the global PSE / pse and Logger tables.
--- --==-==--

local PSE = require("sdk.api")

_G.PSE = PSE
_G.pse = PSE
_G.Logger = PSE.Logger

PSE.Logger.init()

return PSE
