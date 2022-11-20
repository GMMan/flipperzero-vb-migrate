#pragma once

typedef struct VbMigrate VbMigrate;

VbMigrate* vb_migrate_alloc();

void vb_migrate_free(VbMigrate* inst);

void vb_migrate_blink_read(VbMigrate* inst);
void vb_migrate_blink_emulate(VbMigrate* inst);
void vb_migrate_blink_stop(VbMigrate* inst);
