BEGIN;
-- Logout must revoke outstanding one-use GNET tickets instead of failing
-- because game_tickets.session_id referenced a now-revoked web_session.
ALTER TABLE game_tickets DROP CONSTRAINT IF EXISTS game_tickets_session_id_fkey;
ALTER TABLE game_tickets ADD CONSTRAINT game_tickets_session_id_fkey
  FOREIGN KEY(session_id) REFERENCES web_sessions(id) ON DELETE CASCADE;
COMMIT;
