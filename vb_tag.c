#include "vb_tag.h"

#define VB_NAME_VBDM "VB Digital Monster"
#define VB_NAME_VBV "VB Digivice -V-"
#define VB_NAME_VBC "VB Characters"
#define VB_NAME_VH "Vital Hero"

#define VB_NAME_VBDM_SHORT "VBDM"
#define VB_NAME_VBV_SHORT "VBV"
#define VB_NAME_VBC_SHORT "VBC"
#define VB_NAME_VH_SHORT "VH"

static const VbTagProduct vb_tag_valid_products[] = {
    {.item_id = 0x0200, .item_no = 0x0100, .name = VB_NAME_VBDM, .short_name = VB_NAME_VBDM_SHORT},
    {.item_id = 0x0200, .item_no = 0x0200, .name = VB_NAME_VBDM, .short_name = VB_NAME_VBDM_SHORT},
    {.item_id = 0x0200, .item_no = 0x0300, .name = VB_NAME_VBDM, .short_name = VB_NAME_VBDM_SHORT},
    {.item_id = 0x0200, .item_no = 0x0400, .name = VB_NAME_VBV, .short_name = VB_NAME_VBV_SHORT},
    {.item_id = 0x0200, .item_no = 0x0500, .name = VB_NAME_VBV, .short_name = VB_NAME_VBV_SHORT},
    {.item_id = 0x0200, .item_no = 0x0600, .name = VB_NAME_VH, .short_name = VB_NAME_VH_SHORT},
    {.item_id = 0x0300, .item_no = 0x0100, .name = VB_NAME_VBC, .short_name = VB_NAME_VBC_SHORT},
};

BantBlock* vb_tag_get_bant_block(NfcDeviceData* dev) {
    return (BantBlock*)&dev->mf_ul_data.data[16];
}

const VbTagProduct* vb_tag_find_product(const BantBlock* bant) {
    for(size_t i = 0; i < COUNT_OF(vb_tag_valid_products); ++i) {
        const VbTagProduct* product = &vb_tag_valid_products[i];
        if(bant->item_id == product->item_id && bant->item_no == product->item_no) return product;
    }

    return NULL;
}

bool vb_tag_validate_product(NfcDeviceData* dev) {
    // Must be NTAG I2C Plus 1k
    if(dev->protocol != NfcDeviceProtocolMifareUl) return false;
    if(dev->mf_ul_data.type != MfUltralightTypeNTAGI2CPlus1K) return false;
    // Must match one of the known product IDs
    BantBlock* bant = vb_tag_get_bant_block(dev);
    if(bant->magic != BANT_MAGIC) return false;
    return vb_tag_find_product(bant) != NULL;
}

VbTagStatus vb_tag_get_status(const BantBlock* bant) {
    return bant->status;
}

void vb_tag_set_status(BantBlock* bant, VbTagStatus status) {
    bant->status = status;
}

VbTagOperation vb_tag_get_operation(const BantBlock* bant) {
    return bant->operation;
}

void vb_tag_set_operation(BantBlock* bant, VbTagOperation operation) {
    bant->operation = operation;
}
