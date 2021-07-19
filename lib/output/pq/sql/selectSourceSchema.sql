SELECT table_schema, database
FROM mygrate.source s
WHERE s.source_id = $1
