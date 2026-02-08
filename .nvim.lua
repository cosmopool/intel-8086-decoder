local dap = require("dap")

dap.adapters.gdb = {
  id = "gdb",
  type = "executable",
  command = "gdb",
  args = { "--quiet", "--interpreter=dap" },
}

dap.configurations.c = {
  {
    name = "Debug decoder",
    type = "gdb",
    request = "launch",
    program = function()
      os.execute("./build.sh")
      -- vim.cmd("term ./build.sh")
      -- Wait a bit for build to complete
      vim.defer_fn(function()
        vim.notify("Build complete, starting debug...", vim.log.levels.INFO)
      end, 1000)
      return "${workspaceFolder}/decoder"
    end,
    cwd = "${workspaceFolder}",
    stopOnEntry = true,
    args = function()
      local file = vim.fn.input("Path to binary file to decode: ", vim.fn.getcwd() .. "/", "file")
      return { file }
    end,
    externalConsole = false,   -- Critical for seeing output in nvim
    MIMode = 'gdb',
    setupCommands = {
      {
        text = '-enable-pretty-printing',
        description = 'Enable pretty printing',
        ignoreFailures = false,
      },
    },
  },
}
