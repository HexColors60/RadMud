--- File   : utils.lua
--- Brief  : 
--- Author : Enrico Fraccaroli
--- Date   : 29/11/2016
--- Copyright (c) 2016 Enrico Fraccaroli <enrico.fraccaroli@gmail.com>
--- Permission is hereby granted, free of charge, to any person obtaining a
--- copy of this software and associated documentation files (the "Software"),
--- to deal in the Software without restriction, including without limitation
--- the rights to use, copy, modify, merge, publish, distribute, sublicense,
--- and/or sell copies of the Software, and to permit persons to whom the
--- Software is furnished to do so, subject to the following conditions:
--- The above copyright notice and this permission notice shall be included
--- in all copies or substantial portions of the Software.
--- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
--- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
--- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
--- THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
--- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
--- FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
--- DEALINGS IN THE SOFTWARE.
function MessageContains(message, what)
    return string.match(' ' .. message .. ' ', '%W' .. what .. '%W') ~= nil
end

--- Move the character through the given path.
FindPathAndMoveToDestination = function(self, room, minDelay, maxDelay)
    print("Moving to location " .. room.vnum)
    -- Get the path to the given room.
    local path = Mud.findPath(self, room)
    -- If the path is not empty, then move to destination.
    if (not path:empty()) then
        -- Try to move to the destination.
        if (GetToDestination(self, path, minDelay, maxDelay)) then
            return true
        end
    end
    return false
end

--- Move the character through the given path.
-- @param self The character to move.
-- @param path The path to follow.
GetToDestination = function(self, path, minDelay, maxDelay)
    for direction in path:iter() do
        -- Movind to direction.
        if (not self:doCommand(direction:toString())) then
            return false
        end
        Mud.sleep(Mud.random(minDelay, maxDelay))
    end
    return true
end