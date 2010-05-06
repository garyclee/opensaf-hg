#include "cpd.h"

SaAisErrorT cpd_imm_init(CPD_CB *cb);
void cpd_imm_declare_implementer(CPD_CB *cb);
SaAisErrorT create_runtime_ckpt_object(CPD_CKPT_INFO_NODE *ckpt_node, SaImmOiHandleT immOiHandle);
SaAisErrorT create_runtime_replica_object(CPD_CKPT_REPLOC_INFO *ckpt_reploc_node, SaImmOiHandleT immOiHandle);
void cpd_create_association_class_dn(const SaNameT *child_dn, const SaNameT *parent_dn, const char *rdn_tag, SaNameT *dn);
