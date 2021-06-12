SELECT t.table_name, c.column_name, c.mysql_ordinal, k.column_name IS NOT NULL is_pk
FROM mygrate.source s
	JOIN mygrate.tables t USING(source_id)
	JOIN mygrate.table_columns c USING(table_id)
	LEFT OUTER JOIN information_schema.key_column_usage k USING(table_schema, table_name, column_name)
WHERE s.source_id = $1
