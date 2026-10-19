// Run the actual marco.c probe in an older V8 without Wasm/BigInt integration.
// Shell output keeps the probe independent of browser DOM and Node version gates.
const fs = require('fs');
const vm = require('vm');
const context = {
  print: console.log,
  printErr: console.error,
  console,
  performance: require('perf_hooks').performance,
  setTimeout,
  clearTimeout,
  atob: value => Buffer.from(value, 'base64').toString('binary'),
};
vm.runInNewContext(fs.readFileSync(process.argv[2], 'utf8'), context,
  { filename: process.argv[2] });
