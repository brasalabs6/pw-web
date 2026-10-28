BEGIN;
CREATE TABLE IF NOT EXISTS auth_rate_buckets(
  key_sha256 char(64) PRIMARY KEY,
  attempts integer NOT NULL DEFAULT 0 CHECK(attempts>=0),
  bucket_start timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS auth_rate_buckets_start ON auth_rate_buckets(bucket_start);
ALTER TABLE operation_jobs ADD COLUMN IF NOT EXISTS attempts integer NOT NULL DEFAULT 0;
ALTER TABLE operation_jobs ADD COLUMN IF NOT EXISTS reserved_until timestamptz;
CREATE INDEX IF NOT EXISTS operation_jobs_reservation ON operation_jobs(reserved_until) WHERE state='dispatching';
COMMIT;
