import pg from 'pg';
export const connectDb = () => {
  const url = process.env.DATABASE_URL;
  if (!url) throw new Error('DATABASE_URL is required; no in-memory production fallback');
  return new pg.Pool({connectionString:url, max:10, idleTimeoutMillis:10_000,
    ssl: process.env.PG_SSL === 'require' ? {rejectUnauthorized:true} : undefined});
};
export type Db = ReturnType<typeof connectDb>;
