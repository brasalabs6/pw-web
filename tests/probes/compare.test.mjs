import assert from 'node:assert/strict';
import test from 'node:test';
import { compareProbe } from './compare.mjs';

const result = (longBytes) => ({
  schema: 1,
  abi: { longBytes, pointerBytes: longBytes },
  cases: [{ seed: 42, uniform: [0.25, 0.75], gaussian: [1.25, -0.5] }],
});

test('compares numerical behavior while reporting the native/wasm ABI separately', () => {
  assert.deepEqual(compareProbe(result(8), result(4)), { uniform: 2, gaussian: 2 });
});

test('rejects altered samples, truncated output, wrong seeds and invalid numbers', () => {
  for (const mutate of [
    value => { value.cases[0].uniform[0] += 0.001; },
    value => { value.cases[0].uniform.pop(); },
    value => { value.cases[0].seed = 43; },
    value => { value.cases[0].gaussian[0] = NaN; },
    value => { value.cases = []; },
    value => { value.schema = 2; },
  ]) {
    const actual = result(4);
    mutate(actual);
    assert.throws(() => compareProbe(result(8), actual));
  }
});

test('allows only small floating-point differences in Gaussian math', () => {
  const actual = result(4);
  actual.cases[0].gaussian[0] += 1e-14;
  compareProbe(result(8), actual);
  actual.cases[0].gaussian[0] += 1e-5;
  assert.throws(() => compareProbe(result(8), actual));
});

test('rejects empty or malformed native evidence instead of producing a false pass', () => {
  assert.throws(() => compareProbe({}, result(4)));
  assert.throws(() => compareProbe({ ...result(8), cases: [] }, { ...result(4), cases: [] }));
  const empty = result(8);
  empty.cases[0].uniform = [];
  assert.throws(() => compareProbe(empty, empty));
});
