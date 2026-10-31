export function compareProbe(native, wasm) {
  const require = (condition, message) => {
    if (!condition) throw new Error(message);
  };
  for (const value of [native, wasm]) {
    require(value?.schema === 1, 'Unknown probe schema');
    require(Array.isArray(value.cases) && value.cases.length > 0, 'Missing test cases');
  }
  require(native.cases.length === wasm.cases.length, 'Case count differs');
  const counts = { uniform: 0, gaussian: 0 };
  native.cases.forEach((expected, caseIndex) => {
    const actual = wasm.cases[caseIndex];
    require(Number.isInteger(expected.seed) && actual.seed === expected.seed, 'Seed differs');
    for (const kind of ['uniform', 'gaussian']) {
      const a = expected[kind], b = actual[kind];
      require(Array.isArray(a) && a.length > 0 && Array.isArray(b) && a.length === b.length,
        `${kind}: sample count differs or is empty`);
      a.forEach((value, index) => {
        require(Number.isFinite(value) && Number.isFinite(b[index]), `${kind}: non-finite sample`);
        const tolerance = kind === 'uniform' ? 0 : 1e-12 * Math.max(1, Math.abs(value));
        require(Math.abs(value - b[index]) <= tolerance,
          `${kind}: seed ${expected.seed}, sample ${index} differs`);
      });
      counts[kind] += a.length;
    }
  });
  return counts;
}
