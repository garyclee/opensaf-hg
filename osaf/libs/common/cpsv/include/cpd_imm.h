#include "cpd.h"

extern SaAisErrorT cpd_imm_init(SaImmOiHandleT* immOiHandle, SaSelectionObjectT* imm_sel_obj);
extern void cpd_imm_reinit_bg(CPD_CB * cb);
extern void cpd_imm_declare_implementer(SaImmOiHandleT* immOiHandle, SaSelectionObjectT* imm_sel_obj);
extern SaAisErrorT create_runtime_ckpt_object(CPD_CKPT_INFO_NODE *ckpt_node, SaImmOiHandleT immOiHandle);
extern SaAisErrorT delete_runtime_ckpt_object(CPD_CKPT_INFO_NODE *ckpt_node, SaImmOiHandleT immOiHandle);
extern SaAisErrorT create_runtime_replica_object(CPD_CKPT_REPLOC_INFO *ckpt_reploc_node, SaImmOiHandleT immOiHandle);
extern SaAisErrorT delete_runtime_replica_object(CPD_CKPT_REPLOC_INFO *ckpt_reploc_node, SaImmOiHandleT immOiHandle);
extern void cpd_create_association_class_dn(const char *child_dn, const char *parent_dn, const char *rdn_tag, char **dn);
extern SaAisErrorT cpd_clean_checkpoint_objects(CPD_CB *cb);
extern SaUint32T cpd_get_scAbsenceAllowed_attr();
