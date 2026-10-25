import test from 'node:test';
import assert from 'node:assert/strict';
import { createHash } from 'node:crypto';
import { readFileSync } from 'node:fs';
import {
  canonicalBytes,
  canonicalSha256,
  parseCanonicalDocument,
  validateBundleReference,
  validateBundleDependencies,
  validateCatalogReference,
  validateContentGraph,
  validateDocument,
  validateReleaseContent,
} from '../../tools/contracts/manifest-validator.mjs';

const EMPTY_SHA256 = 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855';
const HASH = 'a'.repeat(64);
const SOURCE = 'b'.repeat(40);

const emptyBundle = (id = 'bootstrap/ui', requires = []) => ({
  schema: 1,
  id,
  requires,
  assets: [{
    path: 'ui/empty.bin',
    bytes: 0,
    sha256: EMPTY_SHA256,
    requiredForEntry: true,
    spans: [],
  }],
});

const release = overrides => ({
  schema: 1,
  mode: 'diagnostic',
  sourceClient: '1.5.3',
  sourceCommit: SOURCE,
  upstreamCommit: 'e3d6d0075d0a745c9f9859f2cb25b01d71dec045',
  sourceManifestSha256: HASH,
  toolchainLockSha256: HASH,
  buildProfileSha256: HASH,
  assetSetSha256: HASH,
  assetsSchema: 1,
  protocolProfile: null,
  requiredBrowserFeatures: ['wasm', 'webgl2'],
  runtime: {
    entryModule: 'shell/entry',
    wasmModule: 'runtime/client',
    files: [
      { id: 'shell/entry', sha256: HASH, bytes: 10, mediaType: 'text/javascript' },
      { id: 'runtime/client', sha256: HASH, bytes: 20, mediaType: 'application/wasm' },
    ],
  },
  entryBundles: [],
  ...overrides,
});

test('canonical JSON has sorted keys, no whitespace, and stable SHA-256', () => {
  const actual = canonicalBytes({ z: 1, a: { y: true, b: 'é' } });
  assert.equal(new TextDecoder().decode(actual), '{"a":{"b":"é","y":true},"z":1}');
  assert.equal(createHash('sha256').update(actual).digest('hex'), 'bf94ee4363f386aee231b6fdd47b14d25c8eb263d38f81aef0ee1ed9d2008064');
});

test('canonical parser rejects duplicate keys and noncanonical bytes', () => {
  assert.throws(() => parseCanonicalDocument('{"schema":1,"schema":1}'), /DuplicateKey/);
  assert.throws(() => parseCanonicalDocument('{"outer":{"key":1,"key":2}}'), /DuplicateKey/);
  assert.throws(() => parseCanonicalDocument('{"outer":1,"\\u006futer":2}'), /DuplicateKey/);
  assert.throws(() => parseCanonicalDocument('{ "schema": 1 }'), /NonCanonical/);
  assert.throws(() => parseCanonicalDocument('\uFEFF{}'), /Bom/);
  assert.throws(() => parseCanonicalDocument(`{"x":"${String.fromCharCode(0xd800)}"}`), /InvalidUnicode/);
});

test('accepts canonical diagnostic release and content bundle', async () => {
  const releaseBytes = readFileSync(new URL('../fixtures/manifests/release-diagnostic.json', import.meta.url));
  const bundleBytes = readFileSync(new URL('../fixtures/manifests/content-bundle-bootstrap-ui.json', import.meta.url));
  assert.equal((await validateDocument('release', releaseBytes)).mode, 'diagnostic');
  assert.equal((await validateDocument('bundle', bundleBytes)).id, 'bootstrap/ui');
  assert.equal(createHash('sha256').update(releaseBytes).digest('hex'), '464a4a991352ddd2c0eb7158b889aee23b64a26916082fff40fdfee21cfb735c');
  assert.equal(createHash('sha256').update(bundleBytes).digest('hex'), '9f9c1a71b8f67c1bae03a81792dbf9ddd3d373e889283cf9e7d0b57eac2af73f');
});

test('rejects unknown fields and noncanonical game paths', async () => {
  const unknown = { ...emptyBundle(), typo: true };
  await assert.rejects(validateDocument('bundle', canonicalBytes(unknown)), /UnknownField/);
  const traversal = emptyBundle();
  traversal.assets[0].path = '../outside';
  await assert.rejects(validateDocument('bundle', canonicalBytes(traversal)), /InvalidPath/);
  const repeated = emptyBundle();
  repeated.assets.push({ ...repeated.assets[0], path: 'UI/empty.bin' });
  await assert.rejects(validateDocument('bundle', canonicalBytes(repeated)), /DuplicatePath/);
  const nonNfc = emptyBundle();
  nonNfc.assets[0].path = 'ui/e\u0301.bin';
  await assert.rejects(validateDocument('bundle', canonicalBytes(nonNfc)), /InvalidPath/);
  await assert.rejects(validateDocument('bundle', Buffer.from('{"x":"\\ud800"}')), /InvalidUnicode/);
});

test('rejects span overflow and mismatched reconstructed length', async () => {
  const bundle = emptyBundle();
  bundle.assets[0] = {
    path: 'ui/data.bin', bytes: 2, sha256: HASH, requiredForEntry: true,
    spans: [{ objectSha256: HASH, objectBytes: 4, offset: 3, bytes: 2 }],
  };
  await assert.rejects(validateDocument('bundle', canonicalBytes(bundle)), /SpanOutOfBounds/);
  bundle.assets[0].spans[0].offset = 1;
  bundle.assets[0].bytes = 3;
  await assert.rejects(validateDocument('bundle', canonicalBytes(bundle)), /SpanLengthMismatch/);
});

test('rejects game releases without a pinned protocol profile', async () => {
  await assert.rejects(validateDocument('release', canonicalBytes(release({
    mode: 'game',
  }))), /ProtocolProfileRequired/);
  await assert.rejects(validateDocument('release', canonicalBytes(release({
    mode: 'game', protocolProfile: '../unknown',
  }))), /InvalidPath/);
  await assert.rejects(validateDocument('release', canonicalBytes(release({
    entryBundles: ['login/../escape'],
  }))), /InvalidPath/);
  assert.equal((await validateDocument('release', canonicalBytes(release({
    mode: 'game', protocolProfile: 'realm/approved-v1',
  })))).mode, 'game');
  await assert.rejects(validateDocument('release', canonicalBytes(release({
    runtime: { ...release().runtime, unexpected: true },
  }))), /UnknownField/);
});

test('rejects catalog entries assigned to a different content shard', async () => {
  const catalog = { schema: 1, prefix: '00', bundles: [{ id: 'bootstrap/ui', manifestSha256: HASH }] };
  await assert.rejects(validateDocument('catalog', canonicalBytes(catalog)), /BundleShardMismatch/);
});

test('rejects missing bundle dependencies and cycles', () => {
  assert.throws(() => validateBundleDependencies([emptyBundle('a', ['missing'])]), /MissingDependency/);
  assert.throws(() => validateBundleDependencies([
    emptyBundle('a', ['b']), emptyBundle('b', ['a']),
  ]), /DependencyCycle/);
  assert.doesNotThrow(() => validateBundleDependencies([
    emptyBundle('a'), emptyBundle('b', ['a']),
  ]));
  const longChain = Array.from({ length: 12000 }, (_, index) => ({
    id: `b${index}`,
    requires: index === 0 ? [] : [`b${index - 1}`],
  }));
  assert.doesNotThrow(() => validateBundleDependencies(longChain));
});

test('verifies catalog and bundle hashes, prefix ownership, and global DAG', async () => {
  const fixtureIndex = JSON.parse(readFileSync(new URL('../fixtures/manifests/content-index.json', import.meta.url)));
  const fixtureCatalog = JSON.parse(readFileSync(new URL('../fixtures/manifests/content-catalog-f1.json', import.meta.url)));
  const fixtureBundle = JSON.parse(readFileSync(new URL('../fixtures/manifests/content-bundle-bootstrap-ui.json', import.meta.url)));
  assert.equal(await validateContentGraph({ index: fixtureIndex, catalogs: [fixtureCatalog], bundles: [fixtureBundle] }), true);
  const badHashIndex = structuredClone(fixtureIndex);
  badHashIndex.catalogs[0].sha256 = HASH;
  await assert.rejects(validateContentGraph({ index: badHashIndex, catalogs: [fixtureCatalog], bundles: [fixtureBundle] }), /HashMismatch/);

  const bundles = [
    { ...emptyBundle('bootstrap/a'), assets: [{ ...emptyBundle().assets[0], path: 'ui/a.bin' }] },
    { ...emptyBundle('world/b'), assets: [{ ...emptyBundle().assets[0], path: 'world/b.bin' }] },
  ];
  const prefixFor = id => createHash('sha256').update(id).digest('hex').slice(0, 2);
  const byPrefix = new Map();
  for (const bundle of bundles) {
    const prefix = prefixFor(bundle.id);
    const items = byPrefix.get(prefix) ?? [];
    items.push({ id: bundle.id, manifestSha256: await canonicalSha256(bundle) });
    byPrefix.set(prefix, items);
  }
  const catalogs = [...byPrefix].map(([prefix, items]) => ({
    schema: 1,
    prefix,
    bundles: items.sort((left, right) => left.id.localeCompare(right.id)),
  }));
  const index = {
    schema: 1, sourceClient: '1.5.3', cookerVersion: 'fixture',
    catalogs: await Promise.all(catalogs.map(async catalog => ({ prefix: catalog.prefix, sha256: await canonicalSha256(catalog) }))),
  };
  assert.equal(await validateContentGraph({ index, catalogs, bundles }), true);
  await assert.rejects(validateContentGraph({ index, catalogs: [], bundles }), /CatalogSetMismatch|MissingCatalog/);
  const cycle = bundles.map(bundle => ({ ...bundle, assets: bundle.assets.map(asset => ({ ...asset })) }));
  cycle[0].requires = [cycle[1].id];
  cycle[1].requires = [cycle[0].id];
  const cycleCatalogs = await Promise.all(catalogs.map(async catalog => ({
    ...catalog,
    bundles: await Promise.all(catalog.bundles.map(async item => ({
      id: item.id, manifestSha256: await canonicalSha256(cycle.find(bundle => bundle.id === item.id)),
    }))),
  })));
  const cycleIndex = { ...index, catalogs: await Promise.all(cycleCatalogs.map(async catalog => ({
    prefix: catalog.prefix, sha256: await canonicalSha256(catalog),
  }))) };
  await assert.rejects(validateContentGraph({ index: cycleIndex, catalogs: cycleCatalogs, bundles: cycle }), /DependencyCycle/);
});

test('validates index→catalog→bundle edges without loading the full graph', async () => {
  const index = readFileSync(new URL('../fixtures/manifests/content-index.json', import.meta.url));
  const catalog = readFileSync(new URL('../fixtures/manifests/content-catalog-f1.json', import.meta.url));
  const bundle = readFileSync(new URL('../fixtures/manifests/content-bundle-bootstrap-ui.json', import.meta.url));
  const checkedCatalog = await validateCatalogReference({ index, catalog });
  const checkedBundle = await validateBundleReference({ catalog, bundle });
  assert.equal(checkedBundle.id, 'bootstrap/ui');
  const wrongBundle = structuredClone(checkedBundle);
  wrongBundle.assets[0].requiredForEntry = false;
  await assert.rejects(validateBundleReference({ catalog, bundle: canonicalBytes(wrongBundle) }), /HashMismatch/);
});

test('release pins the exact content graph and entry bundles', async () => {
  const index = JSON.parse(readFileSync(new URL('../fixtures/manifests/content-index.json', import.meta.url)));
  const catalog = JSON.parse(readFileSync(new URL('../fixtures/manifests/content-catalog-f1.json', import.meta.url)));
  const bundle = JSON.parse(readFileSync(new URL('../fixtures/manifests/content-bundle-bootstrap-ui.json', import.meta.url)));
  const manifest = JSON.parse(readFileSync(new URL('../fixtures/manifests/release-diagnostic.json', import.meta.url)));
  assert.equal(await validateReleaseContent({ release: manifest, index, catalogs: [catalog], bundles: [bundle] }), true);
  const wrongSet = { ...manifest, assetSetSha256: HASH };
  await assert.rejects(validateReleaseContent({ release: wrongSet, index, catalogs: [catalog], bundles: [bundle] }), /HashMismatch/);
  const missingEntry = { ...manifest, entryBundles: ['login/missing'] };
  await assert.rejects(validateReleaseContent({ release: missingEntry, index, catalogs: [catalog], bundles: [bundle] }), /MissingBundle/);
});
