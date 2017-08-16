#include <postgres.h>
#include <tcop/deparse_utility.h>

#include "ddl_utils.h"

static AlterTableCmd *
ddl_alter_table_subcmd(CollectedCommand *cmd, int subcmdIndex)
{
	CollectedATSubcmd *sub;
	AlterTableCmd *subcmd;

	Assert(cmd->type == SCT_AlterTable);
	Assert(subcmdIndex < list_length(cmd->d.alterTable.subcmds));
	sub = list_nth(cmd->d.alterTable.subcmds, subcmdIndex);
	subcmd = (AlterTableCmd *) sub->parsetree;
	Assert(IsA(subcmd, AlterTableCmd));
    return subcmd;
}

static inline AlterTableType
ddl_alter_table_subcmd_type(CollectedCommand *cmd, int subcmdIndex)
{
    return ddl_alter_table_subcmd(cmd, subcmdIndex)->subtype;
}

Datum
ddl_change_owner_to(PG_FUNCTION_ARGS)
{
	AlterTableCmd *altersub;
	RoleSpec   *role;
	Name		user = palloc0(NAMEDATALEN);
	CollectedCommand *cmd = (CollectedCommand *) PG_GETARG_POINTER(0);
	int subcmdIndex =  PG_GETARG_INT32(1);

	altersub = ddl_alter_table_subcmd(cmd, subcmdIndex); 

	Assert(IsA(altersub->newowner, RoleSpec));
	role = (RoleSpec *) altersub->newowner;

	memcpy(user->data, role->rolename, NAMEDATALEN);
	PG_RETURN_NAME(user);
}

Datum
ddl_subcmd_name(PG_FUNCTION_ARGS)
{
	AlterTableCmd *altersub;
	Name		name = palloc0(NAMEDATALEN);
	CollectedCommand *cmd = (CollectedCommand *) PG_GETARG_POINTER(0);
	int subcmdIndex =  PG_GETARG_INT32(1);

	altersub = ddl_alter_table_subcmd(cmd, subcmdIndex); 

	if (altersub->name != NULL)
	{
		memcpy(name->data, altersub->name, NAMEDATALEN);
		PG_RETURN_NAME(name);
	}
	else 
	{
		if (altersub->subtype == AT_AddIndex)
		{
		   Node *def = altersub->def;
		   if (IsA(def, IndexStmt)) {
		   	IndexStmt *stmt = (IndexStmt *) def;
			Assert(stmt->isconstraint);
			memcpy(name->data, stmt->idxname, NAMEDATALEN);
			PG_RETURN_NAME(name);
		   }
		}
		if (altersub->subtype == AT_AddConstraint
			|| altersub->subtype == AT_AddConstraintRecurse) 
		{
			Node *def = altersub->def;
		   if (IsA(def, Constraint)) {
				Constraint *stmt = (Constraint *) def;
				memcpy(name->data, stmt->conname, NAMEDATALEN);
				PG_RETURN_NAME(name);
		   }

		}
	
		elog(ERROR, "couldn't find the name");
	}
}

Datum
ddl_is_change_owner(PG_FUNCTION_ARGS)
{
	bool		ret =
	AT_ChangeOwner == ddl_alter_table_subcmd_type(
									(CollectedCommand *) PG_GETARG_POINTER(0), PG_GETARG_INT32(1)
	);

	PG_RETURN_BOOL(ret);
}

Datum
ddl_is_add_constraint(PG_FUNCTION_ARGS)
{
	AlterTableType type = ddl_alter_table_subcmd_type(
									(CollectedCommand *) PG_GETARG_POINTER(0), PG_GETARG_INT32(1)
	);
	/* AddConstraint sometimes transformed to AddIndex if Index is involved. different path than CREATE INDEX. */
	PG_RETURN_BOOL(type == AT_AddConstraint || type ==  AT_AddConstraintRecurse || type == AT_AddIndex);
}

Datum
ddl_is_drop_constraint(PG_FUNCTION_ARGS)
{
	AlterTableType type = ddl_alter_table_subcmd_type(
									(CollectedCommand *) PG_GETARG_POINTER(0), PG_GETARG_INT32(1)
	);
	PG_RETURN_BOOL(type == AT_DropConstraint || type == AT_DropConstraintRecurse);
}

Datum
ddl_alter_table_count_subcmds(PG_FUNCTION_ARGS)
{
	CollectedCommand *cmd = (CollectedCommand *) PG_GETARG_POINTER(0);
	if(cmd->type == SCT_Simple) 
	{
		PG_RETURN_INT32(0);
	}
	Assert(cmd->type == SCT_AlterTable);
	int count = list_length(cmd->d.alterTable.subcmds);
	PG_RETURN_INT32(count);
}
