SELECT column_name, is_nullable = 'YES', column_type, column_key = 'PRI'
FROM information_schema.columns
WHERE table_schema = DATABASE()
AND table_name = ?
ORDER BY ordinal_position
