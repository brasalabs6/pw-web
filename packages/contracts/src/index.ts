import { z } from 'zod';

export const realmStatus = z.enum(['offline', 'maintenance', 'starting', 'online']);
export const releaseStatus = z.enum(['draft', 'validated', 'published', 'revoked']);
export const operationStatus = z.enum(['queued', 'dispatching', 'dispatched', 'running', 'ready', 'failed', 'cancelled', 'blocked']);
export const previewKind = z.enum(['client', 'server', 'paired']);

export const catalogRealm = z.object({
  id: z.string().uuid(),
  slug: z.string(),
  name: z.string(),
  description: z.string(),
  status: realmStatus,
  population: z.number().int().nonnegative().nullable(),
  defaultChannel: z.string()
});
export type CatalogRealm = z.infer<typeof catalogRealm>;

export const registerInput = z.object({
  email: z.email().max(254).transform(v => v.toLowerCase()),
  password: z.string().min(12).max(128),
  displayName: z.string().min(2).max(60)
});
export const loginInput = z.object({email: z.email(), password: z.string()});
export const launcherSessionInput = z.object({
  realmId: z.string().uuid(),
  channel: z.enum(['stable','beta','preview']).default('stable'),
  expectedPairingId: z.string().uuid(),
  expectedClientDigest: z.string().regex(/^[a-f0-9]{64}$/)
});
export const launchReleaseInput = z.object({
  realmId: z.string().uuid(),
  channel: z.literal('stable').default('stable')
});
export type LaunchRelease = {
  schema: 2;
  realmId: string;
  channel: string;
  pairingId: string;
  client: { version: string; releaseDigest: string; manifestUrl: string };
  server: { version: string; releaseDigest: string; deploymentId: string };
  assetBaseUrl: string;
};

export const createRealmInput = z.object({
  slug: z.string().regex(/^[a-z0-9][a-z0-9-]{2,47}$/),
  name: z.string().min(3).max(100),
  description: z.string().max(400).default(''),
  defaultChannel: z.enum(['stable','beta']).default('stable')
});
export const updateRealmInput = z.object({
  name: z.string().min(3).max(100).optional(),
  description: z.string().max(400).optional(),
  status: realmStatus.optional()
}).refine(o => Object.keys(o).length > 0);
export const createPreviewInput = z.object({
  kind: previewKind,
  clientCommit: z.string().regex(/^[0-9a-f]{40}$/).optional(),
  serverCommit: z.string().regex(/^[0-9a-f]{40}$/).optional(),
  realmId: z.string().uuid().optional(),
  ttlHours: z.number().int().min(1).max(72).default(24)
}).superRefine((v,c) => {
  if (v.kind !== 'server' && !v.clientCommit) c.addIssue({code:'custom',message:'clientCommit is required'});
  if (v.kind !== 'client' && !v.serverCommit) c.addIssue({code:'custom',message:'serverCommit is required'});
});
export const patchChannelInput = z.object({
  pairingId: z.string().uuid(),
  expectedRevision: z.number().int().nonnegative()
});
export const pairingCreateInput = z.object({
  clientReleaseId: z.string().uuid(),
  serverReleaseId: z.string().uuid(),
  protocolProfile: z.string().min(3).max(120),
  dataProfile: z.string().min(3).max(120),
  status: z.enum(['untested','preview_only','supported','incompatible']).default('untested'),
  evidenceUrl: z.url().optional()
});
export const releaseCreateInput = z.object({
  version: z.string().min(1).max(80),
  digest: z.string().regex(/^[0-9a-f]{64}$/),
  artifactUrl: z.url().startsWith('https://'),
  protocolProfile: z.string().min(3).max(120),
  dataProfile: z.string().min(3).max(120),
  status: releaseStatus.default('draft')
});

export type LauncherSessionInput = z.infer<typeof launcherSessionInput>;
export type ReleaseSummary = {
  id: string;
  version: string;
  digest: string;
  protocolProfile: string;
  dataProfile: string;
  status: string;
};
export type LaunchBootstrap = {
  schema: 2;
  realmId: string;
  channel: string;
  pairingId: string;
  client: {version: string; releaseDigest: string; manifestUrl: string};
  server: {version: string; releaseDigest: string; deploymentId: string};
  gateway: {url: string; ticket: string; ticketExpiresAt: string};
  assetBaseUrl: string;
};
export const sha256Hex = z.string().regex(/^[0-9a-f]{64}$/);
export const updateManifest = z.object({
  schema: z.literal(2),
  clientVersion: z.string(),
  releaseDigest: sha256Hex,
  assetSetDigest: sha256Hex,
  entry: z.object({js: z.object({url: z.url(), sha256: sha256Hex, bytes: z.number().int().positive()}),
    wasm: z.object({url: z.url(), sha256: sha256Hex, bytes: z.number().int().positive()})}),
  files: z.array(z.object({path: z.string(), url: z.url(), sha256: sha256Hex, bytes: z.number().int().nonnegative()})).max(100000),
  signature: z.object({keyId: z.string(), value: z.string()})
}).strict();
export type UpdateManifest = z.infer<typeof updateManifest>;
export function compatiblePair(client: {protocol_profile:string;data_profile:string},
                               server: {protocol_profile:string;data_profile:string}): boolean {
  return client.protocol_profile === server.protocol_profile &&
         client.data_profile === server.data_profile;
}
