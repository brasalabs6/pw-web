BEGIN;
-- Retry-safe even if the process crashes between SQL COMMIT and checksum insertion.
DO $$
BEGIN
  IF NOT EXISTS(SELECT 1 FROM pg_constraint WHERE conname='operation_jobs_state_check') THEN
    ALTER TABLE operation_jobs ADD CONSTRAINT operation_jobs_state_check
      CHECK(state IN ('queued','dispatching','dispatched','running','ready','failed','cancelled','blocked'));
  END IF;
  IF NOT EXISTS(SELECT 1 FROM pg_constraint WHERE conname='client_releases_status_check') THEN
    ALTER TABLE client_releases ADD CONSTRAINT client_releases_status_check
      CHECK(status IN ('draft','validated','published','revoked'));
  END IF;
  IF NOT EXISTS(SELECT 1 FROM pg_constraint WHERE conname='server_releases_status_check') THEN
    ALTER TABLE server_releases ADD CONSTRAINT server_releases_status_check
      CHECK(status IN ('draft','validated','published','revoked'));
  END IF;
END $$;
CREATE INDEX IF NOT EXISTS game_tickets_active_idx ON game_tickets(expires_at)
  WHERE consumed_at IS NULL;
COMMIT;
