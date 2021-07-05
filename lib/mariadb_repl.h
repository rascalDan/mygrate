#ifndef MYGRATE_MARIADB_REPL_H
#define MYGRATE_MARIADB_REPL_H

// This file exists because mariadb_rpl.h alone fails as it's missing dependencies

#include <cstddef>
#include <mysql.h>

#include <mariadb_rpl.h>

#include <string_view>

inline std::string_view
operator*(const MARIADB_STRING & mbstr)
{
	return {mbstr.str, mbstr.length};
}

#endif
