INSERT INTO mygrate.tables(table_name, source_id)
VALUES($1, $2)
RETURNING table_id
