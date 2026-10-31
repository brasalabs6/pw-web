import CONTENT_SCHEMA from '../assets/schemas/content-manifest.schema.json' with { type: 'json' };
import RELEASE_SCHEMA from '../release/schemas/release-manifest.schema.json' with { type: 'json' };
const EMPTY_SHA256 = 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855';
const UPSTREAM_COMMIT = 'e3d6d0075d0a745c9f9859f2cb25b01d71dec045';
const SCHEMA_KEYWORDS = new Set([
  '$schema', '$id', '$defs', '$ref', 'type', 'const', 'enum', 'required', 'if', 'then', 'allOf',
  'properties', 'additionalProperties', 'items', 'oneOf', 'uniqueItems', 'pattern',
  'minLength', 'maxLength', 'minItems', 'maxItems', 'minimum', 'maximum',
]);

function assertSupportedSchema(schema, root = schema) {
  for (const keyword of Object.keys(schema)) {
    if (!SCHEMA_KEYWORDS.has(keyword)) fail('UnsupportedSchemaKeyword', keyword);
  }
  if (schema.$ref) {
    const target = schema.$ref.slice(2).split('/').reduce((node, key) => node?.[key], root);
    if (!schema.$ref.startsWith('#/') || !target) fail('UnknownSchemaReference', schema.$ref);
  }
  for (const child of Object.values(schema.$defs ?? {})) assertSupportedSchema(child, root);
  for (const child of Object.values(schema.properties ?? {})) assertSupportedSchema(child, root);
  if (schema.items) assertSupportedSchema(schema.items, root);
  for (const child of schema.oneOf ?? []) assertSupportedSchema(child, root);
  for (const child of schema.allOf ?? []) assertSupportedSchema(child, root);
  if (schema.if) assertSupportedSchema(schema.if, root);
  if (schema.then) assertSupportedSchema(schema.then, root);
}
assertSupportedSchema(CONTENT_SCHEMA);
assertSupportedSchema(RELEASE_SCHEMA);

function fail(code, detail = '') {
  throw new Error(detail ? `${code}: ${detail}` : code);
}

function sortJson(value, depth = 0) {
  if (depth > 32) fail('DepthLimit');
  if (value === null || typeof value === 'string' || typeof value === 'boolean') return value;
  if (typeof value === 'number') {
    if (!Number.isSafeInteger(value) || value < 0) fail('InvalidInteger');
    return value;
  }
  if (Array.isArray(value)) return value.map(item => sortJson(item, depth + 1));
  const prototype = typeof value === 'object' ? Object.getPrototypeOf(value) : null;
  if (typeof value !== 'object' || (prototype !== Object.prototype && prototype !== null)) fail('InvalidJsonValue');
  const result = Object.create(null);
  for (const key of Object.keys(value).sort()) result[key] = sortJson(value[key], depth + 1);
  return result;
}

export function canonicalBytes(value) {
  return new TextEncoder().encode(JSON.stringify(sortJson(value)));
}

function bytesFrom(input) {
  if (typeof input === 'string') {
    assertUnicodeScalars(input);
    return new TextEncoder().encode(input);
  }
  if (input instanceof Uint8Array) return input;
  if (input instanceof ArrayBuffer) return new Uint8Array(input);
  fail('InvalidByteInput');
}

function bytesEqual(left, right) {
  if (left.length !== right.length) return false;
  for (let index = 0; index < left.length; index += 1) if (left[index] !== right[index]) return false;
  return true;
}

function assertUnicodeScalars(value) {
  if (typeof value === 'string') {
    for (let index = 0; index < value.length; index += 1) {
      const unit = value.charCodeAt(index);
      if (unit >= 0xd800 && unit <= 0xdbff) {
        const next = value.charCodeAt(index + 1);
        if (!(next >= 0xdc00 && next <= 0xdfff)) fail('InvalidUnicode');
        index += 1;
      } else if (unit >= 0xdc00 && unit <= 0xdfff) fail('InvalidUnicode');
    }
  } else if (Array.isArray(value)) {
    for (const item of value) assertUnicodeScalars(item);
  } else if (value !== null && typeof value === 'object') {
    for (const [key, item] of Object.entries(value)) {
      assertUnicodeScalars(key);
      assertUnicodeScalars(item);
    }
  }
}

function scanString(source, start) {
  let escaped = false;
  for (let i = start + 1; i < source.length; i += 1) {
    const code = source.charCodeAt(i);
    if (escaped) { escaped = false; continue; }
    if (code === 0x5c) { escaped = true; continue; }
    if (code === 0x22) {
      const raw = source.slice(start, i + 1);
      try { return { value: JSON.parse(raw), next: i + 1 }; }
      catch { fail('InvalidJsonString'); }
    }
    if (code < 0x20) fail('InvalidJsonString');
  }
  fail('UnexpectedEnd');
}

function scanValue(source, initial, parentDepth = 0) {
  let i = initial;
  while (/\s/.test(source[i] ?? '')) i += 1;
  const char = source[i];
  if (char === '"') return scanString(source, i).next;
  if (char === '{' || char === '[') {
    const depth = parentDepth + 1;
    if (depth > 32) fail('DepthLimit');
    const object = char === '{';
    const keys = new Set();
    i += 1;
    while (true) {
      while (/\s/.test(source[i] ?? '')) i += 1;
      if (source[i] === (object ? '}' : ']')) return i + 1;
      if (object) {
        if (source[i] !== '"') fail('InvalidJsonObject');
        const parsed = scanString(source, i);
        if (keys.has(parsed.value)) fail('DuplicateKey', parsed.value);
        keys.add(parsed.value);
        i = parsed.next;
        while (/\s/.test(source[i] ?? '')) i += 1;
        if (source[i] !== ':') fail('InvalidJsonObject');
        i = scanValue(source, i + 1, depth);
      } else {
        i = scanValue(source, i, depth);
      }
      while (/\s/.test(source[i] ?? '')) i += 1;
      if (source[i] === (object ? '}' : ']')) return i + 1;
      if (source[i] !== ',') fail('InvalidJsonContainer');
      i += 1;
    }
  }
  const match = /^(?:true|false|null|-?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?)/.exec(source.slice(i));
  if (!match) fail('InvalidJsonValue');
  try { JSON.parse(match[0]); } catch { fail('InvalidJsonValue'); }
  return i + match[0].length;
}

export function parseCanonicalDocument(input, { maxBytes = 2 * 1024 * 1024 } = {}) {
  const bytes = bytesFrom(input);
  if (bytes.length > maxBytes) fail('SizeLimit');
  let source;
  try { source = new TextDecoder('utf-8', { fatal: true, ignoreBOM: true }).decode(bytes); }
  catch { fail('InvalidUtf8'); }
  if (source.startsWith('\uFEFF')) fail('Bom');
  const next = scanValue(source, 0);
  let end = next;
  while (/\s/.test(source[end] ?? '')) end += 1;
  if (end !== source.length) fail('TrailingData');
  let value;
  try { value = JSON.parse(source); } catch { fail('InvalidJson'); }
  assertUnicodeScalars(value);
  if (!bytesEqual(canonicalBytes(value), bytes)) fail('NonCanonical');
  return value;
}

function matchesType(value, expected) {
  const types = Array.isArray(expected) ? expected : [expected];
  return types.some(type => {
    if (type === 'null') return value === null;
    if (type === 'array') return Array.isArray(value);
    if (type === 'object') return value !== null && !Array.isArray(value) && typeof value === 'object';
    if (type === 'integer') return Number.isSafeInteger(value);
    return typeof value === type;
  });
}

function validateSchema(value, schema, root, at = '$') {
  if (schema.$ref) {
    if (!schema.$ref.startsWith('#/')) fail('UnsupportedSchemaReference', schema.$ref);
    const target = schema.$ref.slice(2).split('/').reduce((node, key) => node?.[key], root);
    if (!target) fail('UnknownSchemaReference', schema.$ref);
    return validateSchema(value, target, root, at);
  }
  if (schema.const !== undefined && value !== schema.const) fail('UnsupportedSchema', `${at} must equal ${schema.const}`);
  if (schema.enum && !schema.enum.includes(value)) fail('UnsupportedFeature', `${at} value is not permitted`);
  if (schema.oneOf) {
    let matches = 0;
    for (const candidate of schema.oneOf) {
      try { validateSchema(value, candidate, root, at); matches += 1; }
      catch { /* A nonmatching branch is expected while evaluating oneOf. */ }
    }
    if (matches !== 1) fail('InvalidSchema', `${at} must match exactly one schema`);
  }
  for (const candidate of schema.allOf ?? []) validateSchema(value, candidate, root, at);
  if (schema.if) {
    let conditionMatches = true;
    try { validateSchema(value, schema.if, root, at); }
    catch { conditionMatches = false; }
    if (conditionMatches && schema.then) validateSchema(value, schema.then, root, at);
  }
  if (schema.type && !matchesType(value, schema.type)) fail('InvalidSchema', `${at} has wrong type`);
  if (typeof value === 'string') {
    if (schema.minLength !== undefined && [...value].length < schema.minLength) fail('InvalidSchema', `${at} too short`);
    if (schema.maxLength !== undefined && [...value].length > schema.maxLength) fail('InvalidSchema', `${at} too long`);
    if (schema.pattern && !new RegExp(schema.pattern, 'u').test(value)) fail('InvalidSchema', `${at} pattern`);
  }
  if (typeof value === 'number') {
    if (!Number.isSafeInteger(value) || value < 0) fail('InvalidInteger', at);
    if (schema.minimum !== undefined && value < schema.minimum) fail('InvalidSchema', `${at} below minimum`);
    if (schema.maximum !== undefined && value > schema.maximum) fail('SizeLimit', `${at} above maximum`);
  }
  if (Array.isArray(value)) {
    if (schema.minItems !== undefined && value.length < schema.minItems) fail('InvalidSchema', `${at} needs more items`);
    if (schema.maxItems !== undefined && value.length > schema.maxItems) fail('SizeLimit', `${at} has too many items`);
    if (schema.uniqueItems) {
      const items = value.map(item => JSON.stringify(sortJson(item)));
      if (new Set(items).size !== items.length) fail('InvalidSchema', `${at} has duplicate items`);
    }
    if (schema.items) value.forEach((item, index) => validateSchema(item, schema.items, root, `${at}[${index}]`));
  }
  if (value !== null && typeof value === 'object' && !Array.isArray(value)) {
    for (const key of schema.required ?? []) if (!Object.hasOwn(value, key)) fail('InvalidSchema', `${at} missing ${key}`);
    const properties = schema.properties ?? {};
    for (const key of Object.keys(value)) {
      if (!Object.hasOwn(properties, key)) {
        if (schema.additionalProperties === false) fail('UnknownField', `${at}.${key}`);
      } else validateSchema(value[key], properties[key], root, `${at}.${key}`);
    }
  }
}

async function hashBytes(bytes) {
  if (!globalThis.crypto?.subtle) fail('CryptoUnavailable');
  const digest = new Uint8Array(await globalThis.crypto.subtle.digest('SHA-256', bytes));
  return [...digest].map(byte => byte.toString(16).padStart(2, '0')).join('');
}

async function hashOf(value) {
  return hashBytes(canonicalBytes(value));
}

async function shardForId(id) {
  return (await hashBytes(new TextEncoder().encode(id))).slice(0, 2);
}

function validId(value) {
  if (!/^[a-z0-9][a-z0-9._/-]{0,127}$/.test(value) || value.startsWith('/') || value.includes('//') || value.includes('\\')) return false;
  return value.split('/').every(part => part !== '.' && part !== '..' && part.length > 0);
}

async function validateContent(value, kind) {
  if (kind === 'index') {
    const prefixes = value.catalogs.map(item => item.prefix);
    if (new Set(prefixes).size !== prefixes.length) fail('InvalidSchema', 'DuplicatePrefix');
  }
  if (kind === 'catalog') {
    if (!validId(value.prefix)) fail('InvalidPath', 'catalog prefix');
    const ids = value.bundles.map(bundle => bundle.id);
    if (new Set(ids).size !== ids.length) fail('InvalidSchema', 'DuplicateBundle');
    if (ids.some((id, index) => index > 0 && id <= ids[index - 1])) fail('InvalidSchema', 'BundlesNotSorted');
    for (const bundle of value.bundles) {
      if (!validId(bundle.id)) fail('InvalidPath', bundle.id);
      if (await shardForId(bundle.id) !== value.prefix) fail('InvalidSchema', 'BundleShardMismatch');
    }
  }
  if (kind === 'bundle') {
    if (!validId(value.id)) fail('InvalidPath', value.id);
    for (const required of value.requires) if (!validId(required)) fail('InvalidPath', required);
    const paths = new Set();
    for (const asset of value.assets) {
      const pathValue = asset.path;
      const segments = pathValue.split('/');
      if (new TextEncoder().encode(pathValue).length > 1024 || pathValue !== pathValue.normalize('NFC') ||
          pathValue.startsWith('/') || /^[a-z][a-z0-9+.-]*:/i.test(pathValue) ||
          pathValue.includes('\\') || /[\u0000-\u001f\u007f-\u009f]/u.test(pathValue) ||
          segments.some(part => !part || part === '.' || part === '..')) fail('InvalidPath', pathValue);
      const folded = pathValue.toLocaleUpperCase('und').toLocaleLowerCase('und');
      if (paths.has(folded)) fail('DuplicatePath', pathValue);
      paths.add(folded);
      let total = 0;
      for (const span of asset.spans) {
        if (span.offset + span.bytes > span.objectBytes) fail('SpanOutOfBounds', pathValue);
        total += span.bytes;
        if (!Number.isSafeInteger(total)) fail('InvalidInteger', 'span length overflow');
      }
      if (total !== asset.bytes) fail('SpanLengthMismatch', pathValue);
      if (asset.bytes === 0 && (asset.spans.length !== 0 || asset.sha256 !== EMPTY_SHA256)) fail('InvalidSchema', 'EmptyAssetMismatch');
      if (asset.bytes > 0 && asset.spans.length === 0) fail('InvalidSchema', 'MissingSpans');
    }
  }
}

function validateRelease(value) {
  if (value.protocolProfile !== null && !validId(value.protocolProfile)) fail('InvalidPath', value.protocolProfile);
  for (const id of value.entryBundles) if (!validId(id)) fail('InvalidPath', id);
  const files = new Map();
  for (const file of value.runtime.files) {
    if (!validId(file.id)) fail('InvalidPath', file.id);
    if (files.has(file.id)) fail('InvalidSchema', 'DuplicateRuntimeId');
    files.set(file.id, file);
  }
  const entry = files.get(value.runtime.entryModule);
  const wasm = files.get(value.runtime.wasmModule);
  if (!entry || !wasm) fail('InvalidRelease', 'RuntimeReferenceMissing');
  if (entry.mediaType !== 'text/javascript' || wasm.mediaType !== 'application/wasm') fail('InvalidRelease', 'RuntimeMediaTypeMismatch');
  if (value.mode === 'game' && (typeof value.protocolProfile !== 'string' || !value.protocolProfile)) fail('ProtocolProfileRequired');
  if (value.mode === 'diagnostic' && value.protocolProfile !== null) fail('InvalidRelease', 'DiagnosticProtocolForbidden');
  if (value.sourceCommit.toLowerCase() !== value.sourceCommit || value.upstreamCommit !== UPSTREAM_COMMIT) fail('InvalidRelease', 'SourcePinMismatch');
}

const SCHEMAS = {
  index: [CONTENT_SCHEMA, CONTENT_SCHEMA.$defs.index, 2 * 1024 * 1024],
  catalog: [CONTENT_SCHEMA, CONTENT_SCHEMA.$defs.catalog, 2 * 1024 * 1024],
  bundle: [CONTENT_SCHEMA, CONTENT_SCHEMA.$defs.bundle, 2 * 1024 * 1024],
  release: [RELEASE_SCHEMA, RELEASE_SCHEMA, 1024 * 1024],
};

export async function validateDocument(kind, bytes) {
  const entry = SCHEMAS[kind];
  if (!entry) fail('UnsupportedSchema', kind);
  const [root, schema, maxBytes] = entry;
  const value = parseCanonicalDocument(bytes, { maxBytes });
  if (kind === 'release' && value?.mode === 'game' &&
      (typeof value.protocolProfile !== 'string' || !value.protocolProfile)) fail('ProtocolProfileRequired');
  if (kind === 'release' && value?.mode === 'diagnostic' && value.protocolProfile !== null) {
    fail('InvalidRelease', 'DiagnosticProtocolForbidden');
  }
  validateSchema(value, schema, root);
  if (kind === 'release') validateRelease(value);
  else await validateContent(value, kind);
  return value;
}

export function validateBundleDependencies(bundles) {
  const byId = new Map();
  for (const bundle of bundles) {
    if (byId.has(bundle.id)) fail('InvalidSchema', `DuplicateBundle ${bundle.id}`);
    byId.set(bundle.id, bundle);
  }
  const state = new Map();
  for (const start of byId.keys()) {
    const stack = [[start, false]];
    while (stack.length) {
      const [id, exiting] = stack.pop();
      if (exiting) { state.set(id, 2); continue; }
      if (!byId.has(id)) fail('MissingDependency', id);
      if (state.get(id) === 1) fail('DependencyCycle', id);
      if (state.get(id) === 2) continue;
      state.set(id, 1);
      stack.push([id, true]);
      const requires = byId.get(id).requires;
      for (let index = requires.length - 1; index >= 0; index -= 1) stack.push([requires[index], false]);
    }
  }
}

export async function validateCatalogReference({ index, catalog }) {
  const checkedIndex = await validateDocument('index', index);
  const checkedCatalog = await validateDocument('catalog', catalog);
  const reference = checkedIndex.catalogs.find(item => item.prefix === checkedCatalog.prefix);
  if (!reference) fail('MissingCatalog', checkedCatalog.prefix);
  if (await canonicalSha256(checkedCatalog) !== reference.sha256) fail('HashMismatch', `catalog ${reference.prefix}`);
  return checkedCatalog;
}

export async function validateBundleReference({ catalog, bundle }) {
  const checkedCatalog = await validateDocument('catalog', catalog);
  const checkedBundle = await validateDocument('bundle', bundle);
  const reference = checkedCatalog.bundles.find(item => item.id === checkedBundle.id);
  if (!reference) fail('MissingBundle', checkedBundle.id);
  if (await canonicalSha256(checkedBundle) !== reference.manifestSha256) fail('HashMismatch', `bundle ${reference.id}`);
  return checkedBundle;
}

export async function validateContentGraph({ index, catalogs, bundles }) {
  const checkedIndex = await validateDocument('index', canonicalBytes(index));
  const catalogByPrefix = new Map();
  for (const source of catalogs) {
    const catalog = await validateDocument('catalog', canonicalBytes(source));
    if (catalogByPrefix.has(catalog.prefix)) fail('InvalidSchema', `DuplicateCatalog ${catalog.prefix}`);
    catalogByPrefix.set(catalog.prefix, catalog);
  }
  if (new Set(checkedIndex.catalogs.map(item => item.prefix)).size !== catalogByPrefix.size) {
    fail('InvalidSchema', 'CatalogSetMismatch');
  }
  for (const reference of checkedIndex.catalogs) {
    const catalog = catalogByPrefix.get(reference.prefix);
    if (!catalog) fail('MissingCatalog', reference.prefix);
    if (await canonicalSha256(catalog) !== reference.sha256) fail('HashMismatch', `catalog ${reference.prefix}`);
  }

  const bundleById = new Map();
  for (const source of bundles) {
    const bundle = await validateDocument('bundle', canonicalBytes(source));
    if (bundleById.has(bundle.id)) fail('InvalidSchema', `DuplicateBundle ${bundle.id}`);
    bundleById.set(bundle.id, bundle);
  }
  const catalogBundleIds = new Set();
  for (const catalog of catalogByPrefix.values()) {
    for (const reference of catalog.bundles) {
      if (catalogBundleIds.has(reference.id)) fail('InvalidSchema', `DuplicateBundle ${reference.id}`);
      catalogBundleIds.add(reference.id);
      const bundle = bundleById.get(reference.id);
      if (!bundle) fail('MissingBundle', reference.id);
      if (await canonicalSha256(bundle) !== reference.manifestSha256) fail('HashMismatch', `bundle ${reference.id}`);
    }
  }
  if (catalogBundleIds.size !== bundleById.size) fail('InvalidSchema', 'BundleSetMismatch');
  const ownerByPath = new Map();
  for (const bundle of bundleById.values()) {
    for (const asset of bundle.assets) {
      const key = asset.path.toLocaleUpperCase('und').toLocaleLowerCase('und');
      if (ownerByPath.has(key)) fail('DuplicatePath', `${asset.path} in ${ownerByPath.get(key)} and ${bundle.id}`);
      ownerByPath.set(key, bundle.id);
    }
  }
  validateBundleDependencies([...bundleById.values()]);
  return true;
}

export async function validateReleaseContent({ release, index, catalogs, bundles }) {
  const checkedRelease = await validateDocument('release', canonicalBytes(release));
  const checkedIndex = await validateDocument('index', canonicalBytes(index));
  if (checkedRelease.assetSetSha256 !== await canonicalSha256(checkedIndex)) fail('HashMismatch', 'release assetSetSha256');
  if (checkedRelease.sourceClient !== checkedIndex.sourceClient || checkedRelease.assetsSchema !== checkedIndex.schema) {
    fail('InvalidRelease', 'ContentProfileMismatch');
  }
  await validateContentGraph({ index: checkedIndex, catalogs, bundles });
  const byId = new Map();
  for (const source of bundles) {
    const bundle = await validateDocument('bundle', canonicalBytes(source));
    if (byId.has(bundle.id)) fail('InvalidSchema', `DuplicateBundle ${bundle.id}`);
    byId.set(bundle.id, bundle);
  }
  for (const id of checkedRelease.entryBundles) if (!byId.has(id)) fail('MissingBundle', id);
  validateBundleDependencies([...byId.values()]);
  return true;
}

export async function canonicalSha256(value) {
  return hashOf(value);
}
