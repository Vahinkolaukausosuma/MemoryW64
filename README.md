# MemoryW64
Trash lua memory wrapper 

File tree:

> 64bit_lua.exe

> lua5.1.dll

> memory/core.dll

Example

```lua
m = require"memory.core"
proc = m.FindProcess("PaintDotNet.exe")
m.OpenProcess(proc)
base = m.GetAddress(proc,"PaintDotNet.exe")
MZHeader = string.char(m.ReadByte(base)) .. string.char(m.ReadByte(base+1))
print(MZHeader)
m.CloseHandle()
```
outputs "MZ"

