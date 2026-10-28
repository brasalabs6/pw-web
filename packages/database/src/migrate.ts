import {readFileSync} from 'node:fs';
import {fileURLToPath} from 'node:url';
import {connectDb} from './index.ts';
const db = connectDb();
try {
  const sql = readFileSync(fileURLToPath(new URL('../migrations/001_platform.sql',import.meta.url)), 'utf8');
  await db.query(sql);
  console.log('Migrations applied');
} finally {
  await db.end();
}
