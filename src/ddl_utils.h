#ifndef TIMESCALEDB_DDL_UTILS_H
#define TIMESCALEDB_DDL_UTILS_H
#include <postgres.h>
#include <fmgr.h>

PG_FUNCTION_INFO_V1(ddl_is_change_owner);
PG_FUNCTION_INFO_V1(ddl_is_add_constraint);
PG_FUNCTION_INFO_V1(ddl_is_drop_constraint);
PG_FUNCTION_INFO_V1(ddl_change_owner_to);
PG_FUNCTION_INFO_V1(ddl_alter_table_count_subcmds);
PG_FUNCTION_INFO_V1(ddl_subcmd_name);

#endif   /* TIMESCALEDB_DDL_UTILS_H */
