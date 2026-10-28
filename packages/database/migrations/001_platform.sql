-- Control-plane only. Game WDB/BDB is NEVER modified by this schema.
BEGIN;
CREATE EXTENSION IF NOT EXISTS pgcrypto;
CREATE TABLE IF NOT EXISTS users(
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  email text UNIQUE NOT NULL,
  display_name varchar(60) NOT NULL,
  password_hash text NOT NULL,
  role text NOT NULL DEFAULT 'player' CHECK (role IN ('player','operator','admin')),
  created_at timestamptz NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS web_sessions(
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  user_id uuid NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  token_sha256 char(64) UNIQUE NOT NULL,
  expires_at timestamptz NOT NULL,
  created_at timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS web_sessions_user_idx ON web_sessions(user_id);
CREATE TABLE IF NOT EXISTS realms(
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  slug varchar(48) UNIQUE NOT NULL,
  name varchar(100) NOT NULL,
  description text NOT NULL DEFAULT '',
  status text NOT NULL DEFAULT 'offline' CHECK(status IN ('offline','maintenance','starting','online')),
  default_channel text NOT NULL DEFAULT 'stable',
  population integer,
  created_at timestamptz NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS client_releases(
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  version text NOT NULL, digest char(64) UNIQUE NOT NULL,
  artifact_url text NOT NULL,
  protocol_profile text NOT NULL, data_profile text NOT NULL,
  status text NOT NULL DEFAULT 'draft',
  created_at timestamptz NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS server_releases(
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  version text NOT NULL, digest char(64) UNIQUE NOT NULL,
  artifact_url text NOT NULL, protocol_profile text NOT NULL, data_profile text NOT NULL,
  status text NOT NULL DEFAULT 'draft',
  created_at timestamptz NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS pairings(
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  client_release_id uuid NOT NULL REFERENCES client_releases(id),
  server_release_id uuid NOT NULL REFERENCES server_releases(id),
  protocol_profile text NOT NULL,
  data_profile text NOT NULL,
  status text NOT NULL DEFAULT 'untested' CHECK(status IN ('untested','preview_only','supported','incompatible')),
  evidence_url text,
  UNIQUE(client_release_id,server_release_id)
);
CREATE TABLE IF NOT EXISTS deployments(
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  realm_id uuid NOT NULL REFERENCES realms(id),
  server_release_id uuid NOT NULL REFERENCES server_releases(id),
  gateway_url text NOT NULL,
  asset_base_url text NOT NULL,
  state text NOT NULL DEFAULT 'pending' CHECK(state IN ('pending','starting','ready','degraded','stopped')),
  health_checked_at timestamptz,
  updated_at timestamptz NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS realm_channels(
  realm_id uuid NOT NULL REFERENCES realms(id),
  name text NOT NULL,
  pairing_id uuid REFERENCES pairings(id),
  revision integer NOT NULL DEFAULT 0,
  PRIMARY KEY (realm_id,name)
);
CREATE TABLE IF NOT EXISTS game_tickets(
  token_sha256 char(64) PRIMARY KEY,
  session_id uuid NOT NULL REFERENCES web_sessions(id),
  realm_id uuid NOT NULL REFERENCES realms(id),
  deployment_id uuid NOT NULL REFERENCES deployments(id),
  pairing_id uuid NOT NULL REFERENCES pairings(id),
  expires_at timestamptz NOT NULL,
  consumed_at timestamptz,
  created_at timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS game_tickets_exp_idx ON game_tickets(expires_at);
CREATE TABLE IF NOT EXISTS operation_jobs(
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  kind text NOT NULL,
  state text NOT NULL DEFAULT 'queued',
  payload jsonb NOT NULL,
  result jsonb,
  actor_id uuid NOT NULL REFERENCES users(id),
  idempotency_key text UNIQUE NOT NULL,
  created_at timestamptz NOT NULL DEFAULT now(),
  updated_at timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS operation_jobs_pending_idx ON operation_jobs(created_at) WHERE state='queued';
CREATE TABLE IF NOT EXISTS audit_events(
  id bigserial PRIMARY KEY,
  actor_id uuid REFERENCES users(id),
  action text NOT NULL,
  resource text NOT NULL,
  details jsonb NOT NULL DEFAULT '{}'::jsonb,
  created_at timestamptz NOT NULL DEFAULT now()
);
COMMIT;
