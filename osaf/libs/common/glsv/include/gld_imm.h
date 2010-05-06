#include "gld.h"
#include "immutil.h"
#include "saImm.h"

void gld_imm_declare_implementer(GLSV_GLD_CB *cb);
SaAisErrorT gld_imm_init(GLSV_GLD_CB *cb);
SaAisErrorT create_runtime_object(char *rname, SaTimeT create_time, SaImmOiHandleT immOiHandle);

SaAisErrorT gld_saImmOiRtAttrUpdateCallback(SaImmOiHandleT immOiHandle,
					    const SaNameT *objectName, const SaImmAttrNameT *attributeNames);
