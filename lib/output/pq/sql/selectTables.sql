SELECT t.table_name
FROM mygrate.tables t
WHERE t.source_id = $1
