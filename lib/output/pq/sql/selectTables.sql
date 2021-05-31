SELECT t.table_name, STRING_AGG(k.column_name, '|' ORDER BY k.ordinal_position) pk_cols
FROM mygrate.source s
	JOIN information_schema.tables t USING(table_schema)
	LEFT OUTER JOIN information_schema.key_column_usage k USING(table_schema, table_name)
WHERE s.id = $1
GROUP BY t.table_name
