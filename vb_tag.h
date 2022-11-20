#pragma once

#include <furi.h>
#include <lib/nfc/nfc_device.h>

#define BANT_MAGIC (0x544E4142)

typedef struct {
    uint32_t magic;
    // Note: this should be big endian, but for convenience, we'll treat them as little endian
    uint16_t item_id;
    uint16_t item_no;
    uint8_t status;
    uint8_t dim_no;
    uint8_t operation;
    uint8_t reserved;
    uint8_t app_flag;
    uint8_t padding[3];
} __attribute__((packed)) BantBlock;

typedef struct {
    uint16_t item_id;
    uint16_t item_no;
    const char* name;
} VbTagProduct;

typedef enum {
    VbTagStatusReady = 1 << 0,
    VbTagStatusDimReady = 1 << 1,
} VbTagStatus;

typedef enum {
    VbTagOperationIdle,
    VbTagOperationReady,
    VbTagOperationTransferToApp,
    VbTagOperationCheckDim,
    VbTagOperationReturnFromApp,
    VbTagOperationSpotInit,
    VbTagOperationSpotCommit,
} VbTagOperation;

BantBlock* vb_tag_get_bant_block(NfcDeviceData* dev);
const VbTagProduct* vb_tag_find_product(BantBlock* bant);
bool vb_tag_validate_product(NfcDeviceData* dev);
VbTagStatus vb_tag_get_status(BantBlock* bant);
void vb_tag_set_status(BantBlock* bant, VbTagStatus status);
VbTagOperation vb_tag_get_operation(BantBlock* bant);
void vb_tag_set_operation(BantBlock* bant, VbTagOperation operation);
