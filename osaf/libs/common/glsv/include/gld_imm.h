#include "gld.h"
#include "immutil.h"
#include "saImm.h"

extern void gld_imm_declare_implementer(GLSV_GLD_CB *cb);
extern void gld_imm_reinit_bg(GLSV_GLD_CB * cb);
extern SaAisErrorT gld_imm_init(GLSV_GLD_CB *cb);
extern SaAisErrorT create_runtime_object(char *rname, SaTimeT create_time, SaImmOiHandleT immOiHandle);

extern SaAisErrorT gld_saImmOiRtAttrUpdateCallback(SaImmOiHandleT immOiHandle,
					    const SaNameT *objectName, const SaImmAttrNameT *attributeNames);
