#pragma once

#include <storage/storage.h>

#define VB_MIGRATE_FOLDER ANY_PATH("vb_migrate")

typedef struct VbMigrate VbMigrate;

VbMigrate* vb_migrate_alloc();

void vb_migrate_free(VbMigrate* inst);
