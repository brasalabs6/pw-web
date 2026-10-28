BEGIN;
-- Constrain state machines at the database boundary, not just the UI/Zod layer.
ALTER TABLE operation_jobs ADD CONSTRAINT operation_jobs_state_check
  CHECK(state IN ('queued','dispatching','dispatched','running','ready','failed','cancelled','blocked'));
ALTER TABLE client_releases ADD CONSTRAINT client_releases_status_check
  CHECK(status IN ('draft','validated','published','revoked'));
ALTER TABLE server_releases ADD CONSTRAINT server_releases_status_check
  CHECK(status IN ('draft','validated','published','revoked'));
CREATE INDEX IF NOT EXISTS game_tickets_active_idx ON game_tickets(expires_at)
  WHERE consumed_at IS NULL;
COMMIT;
